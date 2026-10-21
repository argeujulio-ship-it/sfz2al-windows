# Abas de técnicas — v0.4.2

Cada ficha possui quatro abas: **História**, **Golpes simples**, **Super combos**
e **Combos**. Apenas história e retrato dependem do desbloqueio da campanha.
Golpes simples reúne os normais e a referência dos especiais sem barra. Super
combos separa os ataques com gasto de barra que já constavam nas fichas.
Esta é uma seleção inicial de golpes da forma principal, não uma lista exaustiva.

A aba Combos traz uma rota de cancelamento ou ligação por personagem. Ryu,
Ken, Akuma, Adon, Guy, Birdie e Gen também têm uma entrada aérea de dois golpes.
As sequências são instruções para executar manualmente no treino; não são macros.
`c.` indica agachado, `j.` indica salto, `xx` indica cancelamento e `>` indica
o próximo ataque no tempo correto. A legenda dentro de cada aba explica os botões.
Para Dhalsim, o exemplo usa o soco curto com baixo e trás. Gen começa em Mantis.
Nash, Birdie e Bison precisam preparar a carga antes do normal.

## Verificação prática

As 18 rotas de chão foram reproduzidas com entradas determinísticas nos dois
executáveis nativos, Brasil 960813 e Asia 960826. O teste exigiu dano do primeiro
normal antes da entrada do especial e um contador de pelo menos dois acertos.
Rolento e Gen produziram três acertos no caso amostrado. Foram usados adversários
parados, sem defesa, e checkpoints temporários próprios. Nenhum teste iniciou
sessão RetroAchievements ou alterou ROMs, EEPROMs ou progresso do usuário.

As sete entradas aéreas foram conferidas apenas na edição Brasil. O resultado
depende do momento do ataque no salto e da aterrissagem; não há promessa de
funcionar de qualquer distância, contra qualquer personagem ou sob defesa.
Os testes cobrem exemplos curtos, não combos ótimos, dano máximo, variantes EX,
Custom Combos ou todas as condições de combate.

A suíte CTest também verifica, para os 18 IDs, a separação sem perda dos comandos,
a disponibilidade das técnicas com a história bloqueada e a preservação do texto
da biografia. A atualização periódica continua sem reescrever texto idêntico,
mantendo a correção de rolagem da v0.4.1.

## Referências de consulta

- [Guia específico de Zero 2 Alpha, Goh_Billy](https://gamefaqs.gamespot.com/arcade/564272-street-fighter-zero-2-alpha/faqs/54008)
- [Diferenças de Zero 2 Alpha, slateman](https://gamefaqs.gamespot.com/arcade/564272-street-fighter-zero-2-alpha/faqs/28029)

Os textos das instruções e as entradas de teste foram preparados para este app.
Não foram copiadas tabelas completas nem comandos do modo opcional de supers
simplificados de Alpha 2 Gold para consoles. Consultas em 21/10/2026.
