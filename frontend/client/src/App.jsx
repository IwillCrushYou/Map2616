import { useRef, useState } from 'react'
import './App.css'

function App() {
  const [program, setProgram] = useState('MOV R1, 20\nMOV R2, 30\nADD R1, R2\nHALT\n')
  const [output, setOutput] = useState('')
  const [loading, setLoading] = useState(false)
  const moduleFactoryRef = useRef(null)

  const loadModuleFactory = async () => {
    if (moduleFactoryRef.current) return moduleFactoryRef.current

    const importedModule = await import('./wasm/main.js')
    const factory = importedModule.default
    if (!factory) {
      throw new Error('WASM module factory export not found in ./wasm/main.js')
    }

    moduleFactoryRef.current = factory
    return factory
  }

  const runProgram = async () => {
    setLoading(true)
    setOutput('Running...')

    try {
      const createModule = await loadModuleFactory()
      const logs = []

      const module = await createModule({
        print: (text) => logs.push(text),
        printErr: (text) => logs.push(`ERR: ${text}`),
      })

      module.FS.writeFile('/program.txt', program)
      module.callMain(['/program.txt'])

      setOutput(logs.join('\n') || 'Program finished with no output.')
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error)
      setOutput(`Failed to run wasm module.\n${message}`)
    } finally {
      setLoading(false)
    }
  }

  return (
    <main className="app">
      <h1>Map2616 Web Emulator</h1>
      <p className="hint">Paste assembly program text and run it in WebAssembly.</p>

      <textarea
        value={program}
        onChange={(event) => setProgram(event.target.value)}
        className="program-input"
        spellCheck={false}
      />

      <button type="button" onClick={runProgram} disabled={loading}>
        {loading ? 'Running...' : 'Run Program'}
      </button>

      <pre className="output">{output || 'Output will appear here.'}</pre>
    </main>
  )
}

export default App
