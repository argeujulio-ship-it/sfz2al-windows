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

Versão Brasil, 18/10/2026: perfil independente Brazil 960813, chave e 18 ROMs
conferidas; geração própria de código 68000 e Z80, sem reutilizar a tradução Ásia.
Os 18 percursos de personagens foram executados por 6.500 quadros em referência,
nativo e conferência por bloco: hashes de estado iguais e zero fallback 68000/Z80
nas execuções nativas. O WAV de um percurso também coincidiu nos três modos.
Uma execução adicional de 20.000 quadros coincidiu entre nativo e conferência,
com zero fallback nativo; a frase de vitória em português foi observada.
Treino: 4.882 quadros de luta com relógio em 99, 11 reposições de vida em repouso,
danos preservados e prefixo anterior à luta igual ao modo normal.
Os quatro testes da interface passaram. Quatro prévias Brasil foram geradas
somente na instalação local. O seletor separa executável, ROM, EEPROM e prévias;
os controles e ajustes visuais continuam compartilhados.
Esses percursos não cobrem todos os finais, variantes ou controles físicos.

Entrega organizada: o iniciador resolve caminhos relativos ao seu executável e
procura as duas ROMs em user/roms quando ainda não há caminho salvo. A pasta de
fontes fica separada da instalação local, sem copiar backups ou caches de build.
Os testes de interface passaram novamente após esse ajuste de caminhos.

Correção de serviço, 20/10/2026: o problema foi reproduzido com TEST mantido
ligado, retornando ao menu após EXIT. O pulso de seis quadros permite retornar
à inicialização normal em Brasil e Ásia. A entrada via F2 foi observada na
interface recompilada; a saída foi exercitada com replay de navegação em ambas
as edições, usando EEPROMs temporárias. O patch do frontend foi conferido
contra a revisão fixada da base. As ROMs e EEPROMs do usuário não foram alteradas.


Treino aprimorado, 20/10/2026 (v0.2.0): nas duas edições, validados adversário
parado, defesa baixa contra ataques agachados, controle manual, três slots com
120 quadros gravados/reproduzidos exatamente, restauração exata do ponto salvo e
salvamento/carregamento durante reprodução. Validados limite de 1.200 quadros e
leitura de estados antigos versão 7. Cada edição também executou 6.500 quadros
no modo normal com hashes idênticos à v0.1.1. Zero fallback nativo nos percursos
da suíte final. Os quatro testes da interface passaram. A interface do treino
foi observada na edição Brasil. Estes testes não certificam todos os personagens,
trocas de lado, situações de defesa, finais ou controles físicos.
