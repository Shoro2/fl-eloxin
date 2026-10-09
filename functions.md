# Functions

## Summoner (`EloxinSummoner`, GameObject 90001)

`GossipSelect` action 0: if the Eloxin this GameObject summoned is still alive, nothing happens.
Otherwise a player holding one each of items 80011, 80012, 80013 and 80015 loses them and Eloxin
80067 appears at (13421.22, 11432.24, -155.05, o 3.9941) on the GameObject's map
(`TEMPSUMMON_MANUAL_DESPAWN`, corpse delay 5 min); the GameObject turns unselectable and
despawns after 5 s with a 60 s respawn. Missing essences: "You dont have all the Magical
Essence!". Any other action closes the gossip.

## Eloxin (`boss_eloxin`, 80067)

Phases by health: 1 from the pull, 2 below 66 %, 3 below 33 %.

| Phase | Abilities (spell ids from the server `Spell.dbc`) |
|---|---|
| 1 | Poison Nova 550102 every 35 s; Neurotoxin 550103 on two random players every 3-6 s; Sepsis 550104 on the top-threat target every 10 s |
| any | Poison Bolt Volley 550105 when the nearest target is more than 25 yd away |
| 2 | Poison Beam: cast 550101, then 50 ticks of 550100 every 300 ms while turning left or right (texts 5/6), other events paused; repeats; a mushroom every 8 s |
| 3 | also, with at least three candidates, links two random non-tank players within 100 yd every 30 s for 15 s: link 102003 + visual 102006; more than 10 yd apart -> both get Mark of Eloxin 550106, closer -> 102007 |

Spells 102003, 102006 and 102007 exist in neither the server `Spell.dbc` nor
`acore_world.spell_dbc` (2026-10-09), so those casts fail; the 550106 part is real.
Texts: 2 engage is commented out; 7 phase 2, 8 phase 3 ("PH P3"), 1 death. Reset and death remove
all mushrooms within 200 yd.

## Mushrooms (`eloxin_mushroom`)

Summoned on a random player (`TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT`):

| Entry | Behaviour |
|---|---|
| 69702 Green | engages at 1 HP; healed to full -> casts Restore Health 550109 and despawns |
| 69704 Blue | pulses Drain Mana 550110 every 1 s; on death casts Restore Mana 550108 |
| 69703 Red | on death casts Explode 550107 |

A global counter `auraStacks` goes up per summoned mushroom and down per mushroom death; nothing
reads it. Symbiosis 550111 is only removed on reset, never applied.

## Vecna (`boss_vecna`, 9950)

Phases at 95 / 85 / 60 / 30 % health. From the pull it casts Death Sentence 550501 (AoE) every
2 s. Phase 1 summons portal 9951 at (-174.98, -813.81, 41.62), which sends an imp 9953 every
10 s; phase 2 summons portal 9952, which sends a demon 9954 every 35 s; phase 3 schedules a
"pool" whose spell id is 0 (no-op). On death it removes portals, imps and demons within 200 yd.
`vecna_imp` and `vecna_demon` explode on death (the imp's spell id is 0, the demon casts
550500). `spell_vecna_aura` (kill at 100 stacks) exists in the source but is never registered.

## Config

None read. `FLEloxin.Enable` in the conf has no effect.
