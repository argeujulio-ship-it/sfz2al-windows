# Conferência da preparação para GitHub

17/10/2026. Repositório de código-fonte, sem release binária.

- Checkout novo da revisão fixada do CPS-2 Recomp, usando o repositório local
  como origem do clone para este ensaio.
- Patch aplicado sobre esse checkout novo, sem reutilizar fontes traduzidos.
- Código 68000/Z80 gerado novamente com a ROM fornecida localmente. O gerador
  encontrou 80.327 instruções principais com os endereços de cobertura fornecidos.
  Seis decodificações de dados não executáveis foram reportadas pelo gerador.
- Compilação MSVC x64 de sfz2al, sfz2al-check, iniciador e testes concluída.
- CTest: controles e temas passaram, 2/2.
- Execução adicional de 600 quadros em modo nativo e em conferência por bloco:
  hashes de estado iguais; execução nativa reportou zero instruções de fallback.
  Esse ensaio curto cobre inicialização, não toda a partida.
- Auditoria dos arquivos preparados para envio: fontes, metadados de endereços,
  configuração, licenças e três prévias de calibração originais. Não há ROMs,
  chaves, dumps, tradução gerada, executáveis ou credenciais nessa seleção.
- work/, build/ e dist/ foram mantidos fora do índice Git.

Atualização de 18/10/2026: o iniciador recebeu o visual de seleção de luta e
quatro sinais sintetizados originais. Os testes agora são 3/3: menu_audio,
controls e themes. O teste local opcional de reprodução foi aceito pelo Windows.
Na interface foram verificados volume, silêncio, persistência ao reabrir e
abertura da configuração de controles. A mudança não recompila a lógica do jogo.

Foi necessário limitar os caminhos dos objetos de compilação para evitar um
problema de comprimento de nomes no Windows. Isso está no script build.ps1.
Não foi feita validação em uma instalação limpa de outra máquina.

Atualização das prévias, 18/10/2026: os testes passaram em 4/4, incluindo
reprodução GIF, mudança dos pixels, pausa, retomada, troca sem repetição imediata
e funcionamento sem mídia. O teste usa cores sólidas originais; os quatro clipes
de luta foram gerados e validados apenas na instalação local, fora do Git.

Brilho e contraste na prévia: os quatro testes passaram novamente. O teste de
prévia compara os pixels com a transformação compartilhada pelo jogo em três
combinações, verifica a moldura inalterada, o quadro pausado e a restauração exata
das cores originais. Na interface, brilho de 21%, contraste de 79% e restauração
foram observados sobre a mesma cena pausada. A lógica do jogo não foi recompilada.
