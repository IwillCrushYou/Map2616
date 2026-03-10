const OP = {
	ADD: 0x0,
	SUB: 0x1,
	MUL: 0x2,
	DIV: 0x3,
	AND: 0x4,
	OR: 0x5,
	XOR: 0x6,
	NOT: 0x7,
	LOAD: 0x8,
	STOR: 0x9,
	MOV: 0xA,
	JMP: 0xB,
	JZ: 0xC,
	JN: 0xD,
	NOP: 0xE,
	HALT: 0xF
};

const OP_NAMES = Object.entries(OP).reduce((acc, [name, code]) => {
	acc[code] = name;
	return acc;
}, {});

const FLAG_ZERO = 1 << 0;
const FLAG_SIGN = 1 << 1;
const FLAG_CARRY = 1 << 2;
const FLAG_OVERFLOW = 1 << 3;

const MEM_SIZE = 65536;
const NUM_REGS = 16;
const MAX_CYCLES = 10000;
const PROG_START = 0;

const SAMPLE_PROGRAM = [
	"MOV R1, 20",
	"MOV R2, 30",
	"ADD R1, R2",
	"HALT"
].join("\n");

class AssemblerError extends Error {
	constructor(message, lineNo) {
		super(lineNo ? `Line ${lineNo}: ${message}` : message);
	}
}

class CPU {
	constructor() {
		this.memory = new Uint8Array(MEM_SIZE);
		this.regs = new Uint16Array(NUM_REGS);
		this.reset();
	}

	reset() {
		this.regs.fill(0);
		this.PC = PROG_START;
		this.FLAGS = 0;
		this.halted = false;
		this.error = 0;
		this.cycles = 0;
		this.last = { raw: 0, opcode: OP.NOP, dst: 0, src: 0, imm: 0 };
	}

	clearMemory() {
		this.memory.fill(0);
	}

	readWord(addr) {
		if (addr + 1 >= MEM_SIZE) {
			this.halted = true;
			this.error = 3;
			return 0;
		}
		return this.memory[addr] | (this.memory[addr + 1] << 8);
	}

	writeWord(addr, value) {
		if (addr + 1 >= MEM_SIZE) {
			this.halted = true;
			this.error = 3;
			return;
		}
		this.memory[addr] = value & 0xff;
		this.memory[addr + 1] = (value >> 8) & 0xff;
	}

	loadWords(words, addr = PROG_START) {
		for (let i = 0; i < words.length; i++) {
			this.writeWord(addr + i * 2, words[i]);
			if (this.halted) return;
		}
	}

	decode(raw) {
		const opcode = (raw >> 12) & 0xf;
		const dst = (raw >> 8) & 0xf;
		const src = (raw >> 4) & 0xf;
		const imm4 = raw & 0xf;
		const imm8 = raw & 0xff;
		const imm12 = raw & 0x0fff;

		let imm = 0;
		if (opcode >= OP.JMP && opcode <= OP.JN) imm = imm12;
		else if (opcode >= OP.LOAD && opcode <= OP.MOV) imm = imm8;
		else imm = imm4;

		return { opcode, dst, src, imm };
	}

	setFlag(flag, on) {
		if (on) this.FLAGS |= flag;
		else this.FLAGS &= ~flag;
	}

	updateZN(value) {
		const v = value & 0xffff;
		this.setFlag(FLAG_ZERO, v === 0);
		this.setFlag(FLAG_SIGN, (v & 0x8000) !== 0);
	}

	aluAdd(a, b) {
		const full = (a + b) >>> 0;
		const result = full & 0xffff;
		this.setFlag(FLAG_CARRY, full > 0xffff);
		this.setFlag(FLAG_OVERFLOW, ((~(a ^ b) & (a ^ result)) & 0x8000) !== 0);
		this.updateZN(result);
		return result;
	}

	aluSub(a, b) {
		const result = (a - b) & 0xffff;
		this.setFlag(FLAG_CARRY, a < b);
		this.setFlag(FLAG_OVERFLOW, (((a ^ b) & (a ^ result)) & 0x8000) !== 0);
		this.updateZN(result);
		return result;
	}

	aluMul(a, b) {
		const result = (a * b) & 0xffff;
		this.setFlag(FLAG_CARRY, false);
		this.setFlag(FLAG_OVERFLOW, false);
		this.updateZN(result);
		return result;
	}

	aluDiv(a, b) {
		if (b === 0) {
			this.halted = true;
			this.error = 2;
			return 0;
		}
		const result = Math.floor(a / b) & 0xffff;
		this.setFlag(FLAG_CARRY, false);
		this.setFlag(FLAG_OVERFLOW, false);
		this.updateZN(result);
		return result;
	}

	step() {
		if (this.halted) return false;

		const pcBefore = this.PC;
		const raw = this.readWord(this.PC);
		if (this.halted) return false;
		this.PC = (this.PC + 2) & 0xffff;

		const i = this.decode(raw);
		this.last = { raw, ...i, pc: pcBefore };

		switch (i.opcode) {
			case OP.ADD:
				this.regs[i.dst] = this.aluAdd(this.regs[i.dst], this.regs[i.src]);
				break;
			case OP.SUB:
				this.regs[i.dst] = this.aluSub(this.regs[i.dst], this.regs[i.src]);
				break;
			case OP.MUL:
				this.regs[i.dst] = this.aluMul(this.regs[i.dst], this.regs[i.src]);
				break;
			case OP.DIV:
				this.regs[i.dst] = this.aluDiv(this.regs[i.dst], this.regs[i.src]);
				break;
			case OP.AND:
				this.regs[i.dst] = (this.regs[i.dst] & this.regs[i.src]) & 0xffff;
				this.setFlag(FLAG_CARRY, false);
				this.setFlag(FLAG_OVERFLOW, false);
				this.updateZN(this.regs[i.dst]);
				break;
			case OP.OR:
				this.regs[i.dst] = (this.regs[i.dst] | this.regs[i.src]) & 0xffff;
				this.setFlag(FLAG_CARRY, false);
				this.setFlag(FLAG_OVERFLOW, false);
				this.updateZN(this.regs[i.dst]);
				break;
			case OP.XOR:
				this.regs[i.dst] = (this.regs[i.dst] ^ this.regs[i.src]) & 0xffff;
				this.setFlag(FLAG_CARRY, false);
				this.setFlag(FLAG_OVERFLOW, false);
				this.updateZN(this.regs[i.dst]);
				break;
			case OP.NOT:
				this.regs[i.dst] = (~this.regs[i.dst]) & 0xffff;
				this.setFlag(FLAG_CARRY, false);
				this.setFlag(FLAG_OVERFLOW, false);
				this.updateZN(this.regs[i.dst]);
				break;
			case OP.LOAD:
				this.regs[i.dst] = this.readWord(i.imm);
				break;
			case OP.STOR:
				this.writeWord(i.imm, this.regs[i.dst]);
				break;
			case OP.MOV:
				this.regs[i.dst] = i.imm & 0xffff;
				break;
			case OP.JMP:
				this.PC = i.imm & 0x0fff;
				break;
			case OP.JZ:
				if ((this.FLAGS & FLAG_ZERO) !== 0) this.PC = i.imm & 0x0fff;
				break;
			case OP.JN:
				if ((this.FLAGS & FLAG_SIGN) !== 0) this.PC = i.imm & 0x0fff;
				break;
			case OP.NOP:
				break;
			case OP.HALT:
				this.halted = true;
				this.error = 0;
				return false;
			default:
				this.halted = true;
				this.error = 1;
				return false;
		}

		this.cycles += 1;
		if (this.cycles >= MAX_CYCLES) {
			this.halted = true;
			this.error = 4;
			return false;
		}

		return !this.halted;
	}
}

function tokenize(line) {
	return line.split(/[\s,]+/).filter(Boolean);
}

function stripComments(line) {
	const candidates = [line.indexOf("#"), line.indexOf(";"), line.indexOf("//")].filter(i => i >= 0);
	if (candidates.length === 0) return line;
	const cut = Math.min(...candidates);
	return line.slice(0, cut);
}

function parseReg(token, lineNo) {
	if (!/^R\d+$/i.test(token)) {
		throw new AssemblerError(`invalid register '${token}'`, lineNo);
	}
	const idx = Number(token.slice(1));
	if (idx < 0 || idx >= NUM_REGS) {
		throw new AssemblerError(`register out of range '${token}'`, lineNo);
	}
	return idx;
}

function parseNumberOrLabel(token, labels, lineNo) {
	if (/^0x[0-9a-f]+$/i.test(token) || /^\d+$/.test(token)) {
		const value = Number(token);
		if (!Number.isInteger(value) || value < 0 || value > 0xffff) {
			throw new AssemblerError(`value out of range '${token}'`, lineNo);
		}
		return value;
	}
	if (labels.has(token)) return labels.get(token);
	throw new AssemblerError(`unknown label or value '${token}'`, lineNo);
}

function opcodeFromName(name, lineNo) {
	const upper = name.toUpperCase();
	if (!(upper in OP)) throw new AssemblerError(`unknown opcode '${name}'`, lineNo);
	return OP[upper];
}

function assemble(source) {
	const rows = source.split(/\r?\n/).map((text, i) => ({ raw: text, lineNo: i + 1 }));
	const labels = new Map();

	let addr = PROG_START;
	for (const row of rows) {
		const cleaned = stripComments(row.raw).trim();
		if (!cleaned) continue;
		const tokens = tokenize(cleaned);
		if (tokens.length === 0) continue;

		let idx = 0;
		if (tokens[0].endsWith(":")) {
			const label = tokens[0].slice(0, -1).toUpperCase();
			if (!label) throw new AssemblerError("empty label", row.lineNo);
			if (labels.has(label)) throw new AssemblerError(`duplicate label '${label}'`, row.lineNo);
			labels.set(label, addr);
			idx = 1;
		}

		if (idx < tokens.length) {
			opcodeFromName(tokens[idx], row.lineNo);
			addr += 2;
			if (addr > MEM_SIZE) throw new AssemblerError("program exceeds memory", row.lineNo);
		}
	}

	const words = [];
	for (const row of rows) {
		const cleaned = stripComments(row.raw).trim();
		if (!cleaned) continue;
		const tokens = tokenize(cleaned);
		if (tokens.length === 0) continue;

		let idx = 0;
		if (tokens[0].endsWith(":")) idx = 1;
		if (idx >= tokens.length) continue;

		const opcodeName = tokens[idx].toUpperCase();
		const opcode = opcodeFromName(opcodeName, row.lineNo);
		const args = tokens.slice(idx + 1).map(t => t.toUpperCase());

		let word = 0;

		if (opcode === OP.NOP || opcode === OP.HALT) {
			if (args.length !== 0) throw new AssemblerError(`${opcodeName} takes no operands`, row.lineNo);
			word = (opcode & 0xf) << 12;
		} else if (opcode === OP.JMP || opcode === OP.JZ || opcode === OP.JN) {
			if (args.length !== 1) throw new AssemblerError(`${opcodeName} expects 1 operand`, row.lineNo);
			const target = parseNumberOrLabel(args[0], labels, row.lineNo);
			if (target > 0x0fff) throw new AssemblerError("jump target out of range (0..4095)", row.lineNo);
			word = ((opcode & 0xf) << 12) | (target & 0x0fff);
		} else if (opcode === OP.LOAD || opcode === OP.STOR || opcode === OP.MOV) {
			if (args.length !== 2) throw new AssemblerError(`${opcodeName} expects 2 operands`, row.lineNo);
			const dst = parseReg(args[0], row.lineNo);
			const imm = parseNumberOrLabel(args[1], labels, row.lineNo);
			if (imm > 0xff) throw new AssemblerError("immediate/address out of range (0..255)", row.lineNo);
			word = ((opcode & 0xf) << 12) | ((dst & 0xf) << 8) | (imm & 0xff);
		} else if (opcode === OP.NOT) {
			if (args.length !== 1) throw new AssemblerError("NOT expects 1 operand", row.lineNo);
			const dst = parseReg(args[0], row.lineNo);
			word = ((opcode & 0xf) << 12) | ((dst & 0xf) << 8);
		} else {
			if (args.length < 2 || args.length > 3) throw new AssemblerError(`${opcodeName} expects 2 or 3 operands`, row.lineNo);
			const dst = parseReg(args[0], row.lineNo);
			const src = parseReg(args[1], row.lineNo);
			let imm4 = 0;
			if (args.length === 3) {
				const val = Number(args[2]);
				if (!Number.isInteger(val) || val < -8 || val > 15) {
					throw new AssemblerError("imm4 out of range (-8..15)", row.lineNo);
				}
				imm4 = val & 0xf;
			}
			word = ((opcode & 0xf) << 12) | ((dst & 0xf) << 8) | ((src & 0xf) << 4) | (imm4 & 0xf);
		}

		words.push(word & 0xffff);
	}

	return words;
}

function hex(value, width = 4) {
	return `0x${(value >>> 0).toString(16).toUpperCase().padStart(width, "0")}`;
}

const state = {
	cpu: new CPU(),
	words: [],
	log: []
};

const ui = {
	source: document.getElementById("sourceEditor"),
	status: document.getElementById("statusLine"),
	pc: document.getElementById("pcValue"),
	cycle: document.getElementById("cycleValue"),
	halt: document.getElementById("haltValue"),
	error: document.getElementById("errorValue"),
	flagZ: document.getElementById("flagZ"),
	flagS: document.getElementById("flagS"),
	flagC: document.getElementById("flagC"),
	flagV: document.getElementById("flagV"),
	registerGrid: document.getElementById("registerGrid"),
	log: document.getElementById("logOutput"),
	loadSampleBtn: document.getElementById("loadSampleBtn"),
	assembleBtn: document.getElementById("assembleBtn"),
	stepBtn: document.getElementById("stepBtn"),
	runBtn: document.getElementById("runBtn"),
	resetBtn: document.getElementById("resetBtn"),
	clearLogBtn: document.getElementById("clearLogBtn")
};

function renderRegisters() {
	ui.registerGrid.innerHTML = "";
	for (let i = 0; i < NUM_REGS; i++) {
		const card = document.createElement("div");
		card.className = "register";

		const name = document.createElement("span");
		name.className = "name";
		name.textContent = `R${i.toString(16).toUpperCase()}`;

		const value = document.createElement("span");
		value.className = "value";
		value.textContent = hex(state.cpu.regs[i]);

		card.appendChild(name);
		card.appendChild(value);
		ui.registerGrid.appendChild(card);
	}
}

function renderFlags() {
	ui.flagZ.classList.toggle("on", (state.cpu.FLAGS & FLAG_ZERO) !== 0);
	ui.flagS.classList.toggle("on", (state.cpu.FLAGS & FLAG_SIGN) !== 0);
	ui.flagC.classList.toggle("on", (state.cpu.FLAGS & FLAG_CARRY) !== 0);
	ui.flagV.classList.toggle("on", (state.cpu.FLAGS & FLAG_OVERFLOW) !== 0);
}

function renderStatus() {
	ui.pc.textContent = hex(state.cpu.PC);
	ui.cycle.textContent = String(state.cpu.cycles);
	ui.halt.textContent = state.cpu.halted ? "yes" : "no";
	ui.error.textContent = String(state.cpu.error);
	renderFlags();
	renderRegisters();
	ui.log.textContent = state.log.join("\n");
	ui.log.scrollTop = ui.log.scrollHeight;
}

function setStatus(message, isError = false) {
	ui.status.textContent = message;
	ui.status.style.color = isError ? "#ff8f8f" : "#9fb2f6";
}

function loadWordsIntoCpu() {
	state.cpu.reset();
	state.cpu.clearMemory();
	state.cpu.loadWords(state.words, PROG_START);
}

function assembleAndLoad() {
	const src = ui.source.value;
	state.words = assemble(src);
	loadWordsIntoCpu();
	state.log.push(`Assembled ${state.words.length} instruction(s).`);
	setStatus("Assembled successfully.");
	renderStatus();
}

function logLastStep() {
	const last = state.cpu.last;
	const opName = OP_NAMES[last.opcode] || "???";
	const line = `[${String(state.cpu.cycles).padStart(4, "0")}] PC=${hex(last.pc)} raw=${hex(last.raw)} ${opName.padEnd(4, " ")} dst=R${last.dst.toString(16).toUpperCase()} src=R${last.src.toString(16).toUpperCase()} imm=${hex(last.imm)}`;
	state.log.push(line);
}

function stepCpu() {
	const keepRunning = state.cpu.step();
	logLastStep();
	if (!keepRunning && state.cpu.halted) {
		state.log.push(`CPU halted. error=${state.cpu.error}`);
		setStatus(`CPU halted (error=${state.cpu.error}).`, state.cpu.error !== 0);
	} else {
		setStatus("Executed one step.");
	}
	renderStatus();
}

function runCpu() {
	let steps = 0;
	while (!state.cpu.halted && steps < MAX_CYCLES) {
		const keepRunning = state.cpu.step();
		logLastStep();
		steps += 1;
		if (!keepRunning) break;
	}

	if (state.cpu.halted) {
		setStatus(`Run complete. halted=yes error=${state.cpu.error}.`, state.cpu.error !== 0);
	} else {
		setStatus("Run stopped at cycle limit.", true);
	}

	renderStatus();
}

function resetCpu() {
	loadWordsIntoCpu();
	state.log.push("CPU reset.");
	setStatus("CPU reset.");
	renderStatus();
}

function bindEvents() {
	ui.loadSampleBtn.addEventListener("click", () => {
		ui.source.value = SAMPLE_PROGRAM;
		setStatus("Sample program loaded.");
	});

	ui.assembleBtn.addEventListener("click", () => {
		try {
			assembleAndLoad();
		} catch (err) {
			setStatus(err.message, true);
			state.log.push(`Assembler error: ${err.message}`);
			renderStatus();
		}
	});

	ui.stepBtn.addEventListener("click", () => {
		try {
			if (state.words.length === 0) assembleAndLoad();
			if (state.cpu.halted) {
				setStatus("CPU is halted. Reset to run again.", true);
				return;
			}
			stepCpu();
		} catch (err) {
			setStatus(err.message, true);
			state.log.push(`Error: ${err.message}`);
			renderStatus();
		}
	});

	ui.runBtn.addEventListener("click", () => {
		try {
			if (state.words.length === 0) assembleAndLoad();
			if (state.cpu.halted) {
				setStatus("CPU is halted. Reset to run again.", true);
				return;
			}
			runCpu();
		} catch (err) {
			setStatus(err.message, true);
			state.log.push(`Error: ${err.message}`);
			renderStatus();
		}
	});

	ui.resetBtn.addEventListener("click", () => {
		if (state.words.length === 0) {
			setStatus("Assemble a program first.", true);
			return;
		}
		resetCpu();
	});

	ui.clearLogBtn.addEventListener("click", () => {
		state.log = [];
		renderStatus();
	});
}

function init() {
	ui.source.value = SAMPLE_PROGRAM;
	bindEvents();
	renderStatus();
}

init();
