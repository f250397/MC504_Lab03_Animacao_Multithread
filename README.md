# MC504_Lab03_Animacao_Multithread

O programa multithread proposto foi inspirado no problema H2O do livro e ele se trata de um problema de fila de montagem de carros. Para tanto, considera que 1 carro é montado por 1 chassi, 1 motor e 4 rodas e cada uma dessas partes representa uma thread no programa. Quando houver todas as partes na fila de espera, inicia o processamento das partes em um carro e, após o carro ser montado, libera a fila de montagem para receber novas peças e montar outro carro.

Foram utilizadas tanto mutexes, semáforos e esperas busy-wait a fim de solucionar o problema. Ademais, criou-se 3 chamadas de peças que simulam a chegada de peças em tempos aleatórios. No fim, tal abordagem não foi usada ao seu máximo, pois não são apresentadas quantas peças estão atualmente na fila, porém seria possível realizar tal feito ao considerar que as peças na fila seriam representadas pelo número de threads daquela parte atualmente existentes.

As seguintes LLMs foram utilizadas para a resolução do projeto:
https://claude.ai/share/882e0a43-927a-4399-90a9-90998d415214
https://share.gemini.google/Spal9ObDuLED

Em geral, elas foram utilizadas para resolver problemas com o github, problemas com a execução do código e para identificar algumas condições de corrida durante a construção do código. Analisando criticamente, elas foram úteis para a identificação dessas condições de corrida, tal como a existência do mutex logo após o busy-wait em função "roda" anteriormente gerava alguns problemas e passou despercebido. Isso ocorreu, pois o problema estava em quando o semáforo era liberado e a thread liberada adicionava também um valor +1 a condicional do busy-wait antes de todas threads "roda" do "carro" anterior serem finalizadas, causando um comportamento não previsto que afetava toda linha de montagem. Ademais, o Gemini em específico foi usado para obter as imagens ASCII, invés de fazê-las à mão. Em geral, o Claude foi útil para verificar alguns bugs que passaram despercebidos, e.g:

Claude:
Isso bate exatamente com o bug que identifiquei antes — a assimetria entre lock() condicional e unlock() incondicional. Vou detalhar por que ela produz exatamente esse padrão (algumas prints saem, outras threads ficam presas).
(...)

Bug ao realizar o "mutex_lock" dentro de condicional, mas "mutex_unlock" fora, fazendo que threads que não realizaram "lock", realizem "unlock" e resultando em comportamento indefinido.
