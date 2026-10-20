# RetroAchievements — v0.4.0 experimental

Integração opcional com o cliente oficial rcheevos, somente **Softcore**.
Ela está desligada por padrão. A galeria local continua independente da conta.

## Como usar

1. Abra **Conquistas online** no iniciador.
2. Digite seu usuário e senha do RetroAchievements no próprio app e clique em Entrar.
3. Aguarde a lista e marque **Ativar Softcore ao jogar: enviar conquistas para minha conta**.
4. Feche qualquer jogo já aberto e inicie uma nova sessão por **JOGAR**.

Não envie senha ou token pelo chat. Não é necessária uma chave Web API.
A consulta da lista não concede conquistas. Durante o jogo, avisos na parte inferior
informam conexão, desbloqueios e suspensão. Confira também seu perfil no serviço.
Internet e uma conta em [RetroAchievements](https://retroachievements.org) são necessários.

São usados os perfis completos e validados por CRC de sfz2al (Asia 960826) e sfz2alb
(Brazil 960813), associados ao [jogo 11832](https://retroachievements.org/game/11832).
O hash arcade segue o nome canônico do conjunto, após validar a ROM, e não o nome
livre escolhido para o ZIP. Os hashes publicados são:

| Perfil | MD5 do identificador canônico |
|---|---|
| sfz2al | 509cf3b79fa57d7a13217f32efbdc0c4 |
| sfz2alb | 0ccb683be8343aed11f3464be25fe439 |

## Regras e estados salvos

- Treino, execução sem janela, reprodução de entradas, pesquisa e alterações por
  linha de comando não iniciam uma sessão de conquistas.
- Abrir o menu de serviço ou ativar assistência de vida/tempo pausa conquistas
  até fechar e reabrir o jogo pelo iniciador.
- Desmarcar a opção ou sair da conta interrompe novas avaliações no jogo aberto
  na próxima verificação, realizada a cada segundo durante a execução de quadros.
  Envios legítimos que já estavam em andamento podem terminar. Feche o jogo para
  encerrar completamente aquela sessão. Trocar de conta também exige reabertura.
- Estados novos, formato 9, incluem o progresso do avaliador para a mesma conta
  e revisão. Estados antigos, formatos 5 a 8, continuam legíveis. Carregá-los com
  conquistas ativas pausa o serviço, pois não contêm esse progresso compatível.
- Carregar estado antes de concluir a conexão também pausa o serviço. Reinicie
  pelo iniciador para voltar. Executáveis antigos não leem o novo formato 9.

Não há Hardcore, placares competitivos ou importação dos desbloqueios da galeria.
O modo Softcore permite estados salvos; isso não significa autorização para editar
RAM e enviar conquistas artificiais. Os testes sintéticos nunca usam uma conta real.

## Conta e privacidade

A senha não é salva em arquivo. O app guarda usuário e token em
`user/retroachievements/account.bin`, protegidos com DPAPI para o usuário atual
do Windows. Esse arquivo não é portátil para outras contas/instalações do Windows.
**Sair da conta** desativa o envio e remove o token local; não exclui a conta online.
Não publique a pasta user nem seus estados salvos: podem identificar sua conta.
Não há coleta própria de telemetria nem envio dos arquivos da ROM. O cliente envia
ao RetroAchievements os dados de autenticação/sessão, identificador do jogo e
eventos necessários ao serviço, usando HTTPS. A preferência é guardada em
launcher.ini e a autorização da sessão em user/retroachievements/online.enabled.

## Validação e limites conhecidos

Passaram seis testes CTest, incluindo uma conquista criada exclusivamente para
teste offline, login e transporte simulados, ausência de envios na consulta,
desativação de sessão, proteção DPAPI e serialização sem duplicar o desbloqueio.
O teste de memória confere IDs de personagem, vida e estado de final nos offsets
usados pelo serviço. Consultas HTTPS públicas reais reconheceram ambas as ROMs
como jogo 11832 e retornaram as notas públicas de memória.

Em cada edição, 6.500 quadros do modo normal mantiveram os hashes da v0.3.0.
Estados antigos formato 8 e gravações de treino no formato 9 foram conferidos.
Esses percursos ainda possuem fallback para o interpretador em alguns destinos;
a igualdade com a versão anterior não certifica tradução completa ou fidelidade
de todos os percursos.

**Ainda não foi realizado login real nem enviado um desbloqueio real nesta
integração.** As 87 regras oficiais não foram validadas uma a uma. A confirmação
de uma primeira conquista pelo usuário e a verificação das demais regras são
etapas pendentes. Esta é uma integração experimental; não há aprovação oficial
do aplicativo pelo RetroAchievements nem promessa de compatibilidade Hardcore.

## Implementação e referências

Biblioteca rcheevos fixada no commit
`dfdc64ea035c34aa168cf13bbda72b71ca4eba1e`; código upstream preservado, com
um arquivo CMake local para compilação. O hospedeiro Windows usa WinHTTP em
trabalhadores e executa os callbacks/avaliação na thread principal.
A RAM de 64 KiB é exposta em offsets, convertendo os bytes do formato 68000
para o buffer de palavras trocadas usado pelo FBNeo: endereço de byte XOR 1.

- [Integração rc_client](https://github.com/RetroAchievements/rcheevos/wiki/rc_client-integration)
- [Código e licenças rcheevos](https://github.com/RetroAchievements/rcheevos/tree/dfdc64ea035c34aa168cf13bbda72b71ca4eba1e)
- [Hashes reconhecidos](https://retroachievements.org/game/11832/hashes)
- [Mapa de memória FBNeo](https://github.com/libretro/FBNeo/blob/master/src/burner/libretro/retro_memory.cpp)
- [Interface 68000 FBNeo](https://github.com/libretro/FBNeo/blob/master/src/cpu/m68000_intf.cpp)

Não são distribuídos dados de conta, notas completas do serviço ou regras de
conquistas copiadas. A lista e suas descrições são consultadas pelo cliente.
