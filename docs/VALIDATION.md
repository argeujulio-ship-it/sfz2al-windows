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

Foi necessário limitar os caminhos dos objetos de compilação para evitar um
problema de comprimento de nomes no Windows. Isso está no script build.ps1.
Não foi feita validação em uma instalação limpa de outra máquina.
