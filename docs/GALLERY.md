# Galeria — v0.3.0 experimental

Abra **Galeria** no iniciador. Na primeira abertura, o app prepara os 18 retratos
a partir da sua ROM completa da edição selecionada. Não precisa instalar Python.
A captura roda em segundo plano, usando EEPROM e configurações temporárias;
não usa a memória da sua placa nem altera seus controles.

As abas **Golpes simples**, **Super combos** e **Combos** ficam disponíveis desde
o início. A biografia fica na aba **História**. Consulte as sequências e a
validação em [GALLERY_COMBOS.md](GALLERY_COMBOS.md). As imagens, origem,
estilo e breve história são liberados ao vencer o Arcade com a forma principal
de cada personagem. Ao chegar a **18 / 18**, clique em **Ver recompensa completa**:
o app compõe um painel dos 18 retratos originais e salva `user/gallery/recompensa.png`.
É uma composição local de retratos; não é uma ilustração coletiva inédita da Capcom.

O progresso é compartilhado entre Brasil e Ásia, guardado em `user/gallery/progress`.
Preserve a pasta `user` ao atualizar. Os retratos são separados por edição nas pastas
`user/gallery/asia` e `user/gallery/brasil`. Nenhum desses arquivos deve ser publicado
com o código ou incluído no pacote de executáveis.

As conquistas começam a ser registradas nesta versão; campanhas antigas não podem
ser deduzidas da EEPROM. Treino, demonstração, versus, modos alternativos, assistência
de vida/tempo e o menu de serviço não concedem o desbloqueio. Pode continuar uma
campanha após perder; é o personagem que alcança o final que recebe o crédito.
As variantes secretas não entram na contagem dos 18. Carregar apenas um estado já
no final não concede a ficha: o observador precisa ver combate solo antes do final.
Esse é um progresso local, sem conta online, sincronização ou proteção contra edição.

As fichas agora incluem história, motivação e relações de cada um dos 18 lutadores.
As referências e o recorte editorial estão em [GALLERY_STORIES.md](GALLERY_STORIES.md).
A atualização de progresso preserva a posição de leitura quando a ficha não muda.

Os resumos são textos próprios em português e os comandos são uma referência dos
golpes principais da forma padrão. Não incluem todos os golpes, variantes ou detalhes
de execução. As setas assumem que o lutador olha para a direita. Colchetes indicam
carga; P/K indicam soco/chute; PPP/KKK são os três botões juntos. Supers exigem barra.

## Implementação e validação

O observador lê a hierarquia de estados em FF8000/FF8004/FF8008/FF800C, o modo
FF8138, a presença de jogadores em FF80A8 e os campos de personagem/CPU dos dois
atores. Arma durante combate humano solo e confirma a entrada no final (game=10)
em dois quadros consecutivos. Não escreve RAM nem entra no estado serializado.
O estado do observador é descartado ao carregar um save; o progresso já obtido é
persistente e não é revertido por F9. A opção explícita `--gallery-dir` habilita a
gravação; o iniciador a passa apenas ao modo normal. Marcadores independentes evitam
reescrever todo o histórico ao liberar uma ficha.

Foi observado o final de Nash em Brasil e Ásia com o recurso de pesquisa de vida
controlada, separado da distribuição. Na versão Brasil ampliada, 30.000 quadros
coincidiram entre tradução nativa e referência por blocos, com zero fallback principal
e de áudio; Nash foi liberado em ambos. A tradução Brasil foi ampliada com os destinos
adicionais observados nesse percurso. Isso não certifica campanhas normais dos 18
personagens nem todos os chefes, variantes e finais.

Testes sem ROM exercitam os 18 IDs, persistência/idempotência, jogador 2 e rejeição
de treino, CPU, demonstração, versus, variantes, modos alternativos e final isolado.
A interface foi conferida com progresso zero, uma ficha e 18 fichas em uma pasta
de teste. O caso 18/18 usa marcadores sintéticos para conferir a recompensa; não
representa 18 campanhas vencidas. Os retratos gerados foram validados pelo ID da
seleção para os 18 personagens. A recompensa foi salva e inspecionada localmente.
O modo normal preservou os hashes de 6.500 quadros de entradas solo automáticas
por edição em comparação com a v0.2.0; treino não gerou marcadores. Os cinco testes
CTest passaram. Nenhum desbloqueio artificial foi copiado para a instalação do usuário.

## Referências editoriais

- [Manual original publicado pela Capcom](https://captown.capcom.com/en/classic_games/41)
- [Manual de Alpha 2 para PlayStation](https://www.videogamemanual.com/ps1/Street%20Fighter%20Alpha%202%20%28USA%29.pdf)
- [Goh_Billy: observações e comandos específicos de Zero 2 Alpha](https://gamefaqs.gamespot.com/arcade/564272-street-fighter-zero-2-alpha/faqs/54008)
- [Kao_Megura: comandos de Alpha 2](https://gamefaqs.gamespot.com/arcade/583633-street-fighter-alpha-2/faqs/5208)
- [C. Blaney: guia original de Alpha 2 arcade](https://gamefaqs.gamespot.com/arcade/583633-street-fighter-alpha-2/faqs/891)

Não foram copiados os textos, tabelas completas ou imagens desses materiais. As
imagens usadas pela galeria vêm exclusivamente da ROM local do usuário.
