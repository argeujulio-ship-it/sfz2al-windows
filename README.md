# SFZ2AL Windows — experimental

Adaptação para Windows de Street Fighter Zero 2 Alpha (Asia 960826), baseada no
[CPS-2 Recomp](https://github.com/tmcleroy/cps2recomp). Este repositório contém
código-fonte e ajustes; o usuário fornece seu próprio conjunto sfz2al.zip.

## Recursos

- Iniciador em português, seleção do ZIP e modos Jogar/Treino.
- Configuração de teclado e gamepad para dois jogadores, incluindo 3 socos e
  3 chutes em um botão.
- Temas Imagem limpa, Fliperama, TV de tubo e Gabinete clássico.
- Brilho e contraste ajustáveis com o jogo aberto, com restauração dos padrões.
- F8 alterna os temas da sessão quando não está atribuído a uma ação; F11 troca
  para tela cheia.

![Gabinete clássico com grade original](docs/previews/tema_gabinete_classico.png)

## Conteúdo e limites

Não há ROMs, chaves, programa traduzido, imagens ou áudio do jogo, nem binários.
O nome de uma chave em game.cfg é apenas um identificador com checksum.
Os endereços de entrada são metadados de cobertura, sem instruções.

A tradução 68000/Z80 é produzida localmente a partir da ROM fornecida. Caminhos
ainda não traduzidos podem usar o interpretador. Não é uma reimplementação
independente de toda a lógica do jogo, nem há certificação de fidelidade total.
O treino básico não impede um nocaute. O analógico esquerdo mantém o movimento;
os controles são atribuídos pela ordem em que o jogo os reconhece. Ainda faltam
validação ampla de controles físicos, variantes, todos os finais e áudio.

## Compilação local

Requer Windows x64, Git, Python 3, CMake 3.28 ou superior, Ninja e MSVC com Windows
SDK. Use um Developer PowerShell do Visual Studio. Também são necessários:

- SDL 3.4.16, pacote de desenvolvimento Visual C++ x64:
  https://github.com/libsdl-org/SDL/releases/tag/release-3.4.16
- Fonte da zlib 1.3.1: https://github.com/madler/zlib/tree/v1.3.1
- Seu conjunto sfz2al.zip, identificado pelo perfil Asia 960826.

Na raiz deste repositório:

    ./scripts/bootstrap.ps1
    ./scripts/build.ps1 -Rom "D:/Jogos/sfz2al.zip" -SdlDir "D:/deps/SDL3-3.4.16" -ZlibSource "D:/deps/zlib"

Bootstrap baixa apenas o toolkit na revisão fixada e aplica o patch. Não baixa
o jogo e não modifica um checkout que já exista. Build compila primeiro um
preparador com interpretador, gera o código local, recompila com a tradução
nativa e executa os testes de controles e apresentação. O uso do interpretador
nessa preparação não é apresentado como tradução nativa concluída.

O resultado fica em dist/. Abra SFZ2AL.exe e selecione seu ZIP; não há caminho de
ROM pessoal embutido. As configurações são guardadas junto ao executável.
Não publique work/ ou dist/: eles contêm código produzido a partir do jogo.
Não há release binária neste repositório.

## Testes sem ROM

Depois de bootstrap, com o SDK SDL disponível:

    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSDL3_DIR="D:/deps/SDL3-3.4.16/cmake"
    cmake --build build
    Copy-Item "D:/deps/SDL3-3.4.16/lib/x64/SDL3.dll" build/
    ctest --test-dir build --output-on-failure

Esses testes verificam configuração e salvamento dos dois jogadores, ações
simultâneas, recuperação de falhas, molduras e transformação de cores. Eles
não substituem validação de uma partida completa.

## Licença

Os resultados do ensaio de compilação e seus limites estão em
[docs/VALIDATION.md](docs/VALIDATION.md).

GPL-3.0-only para os fontes e ajustes deste projeto. Veja [LICENSE](LICENSE),
[NOTICE.md](NOTICE.md) e [UPSTREAM_NOTICES.md](UPSTREAM_NOTICES.md).
