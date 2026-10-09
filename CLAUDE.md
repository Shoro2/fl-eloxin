# fl-eloxin

Two scripted legacy bosses from the old Forgotten Land fork. **Eloxin, The Cursed King** (80067)
is a three-phase poison boss on the Azealia hub map 727: a player who brings one Magical
Essence each of Air, Fire, Earth and Shadow to the "Eloxin imprisonment vial" (GameObject 90001)
summons him. **Vecna** (9950) is a portal-and-adds boss, ported in July 2026 but never spawned,
on the old server or now. There is no module README (`.github/README.md` is the AzerothCore
skeleton text).

Content chain (FL world DB, workbench checked 2026-10-09): Magical Powder 80010 drops from the
Arcane Magical Anomalies; 100 powder at fl-MagicalRuine's Magical Ruin summon an elemental boss
(80022-80026), each dropping one essence (80011-80015); four essences summon Eloxin here.

## Ids and tables

| What | Id / name |
|---|---|
| Eloxin | creature 80067 "Eloxin" `<The Cursed King>`, ScriptName `boss_eloxin`, loot 55555, `creature_text` groups 1-8 |
| Summoner | GameObject 90001 "Eloxin imprisonment vial", ScriptName `EloxinSummoner`, gossip menu 51001; spawn guid 3501987 (map 727) |
| Essences used | items 80011 Air, 80012 Fire, 80013 Earth, 80015 Shadow (one each; Water 80014 is not used) |
| Mushrooms | 69702 Green, 69703 Red, 69704 Blue Slime, ScriptName `eloxin_mushroom` (the FL DB also binds 69701 and 69705) |
| Eloxin spells | 550100-550111 (server `Spell.dbc`); phase-3 link 102003, 102006, 102007 |
| Vecna | 9950 `boss_vecna`; portals 9951/9952; imp 9953 `vecna_imp`; demon 9954 `vecna_demon`; spells 550500, 550501 |

The spell band 550000-559999 is shared with the CoA package (share-public `06-custom-ids.md`,
"Residual exposure"): check every carrier before adding an id there.

Tables: none. The module's only SQL is the skeleton's `acore_string` row 35410 ("Hello World from
Skeleton-Module!"). All creatures, the GameObject, items, texts and loot are FL world content
from the July 2026 migration; the Vecna ScriptNames were bound by share-public
`python_scripts/fl_content/sql/42_fl_boot_loot_and_script_names.sql` (MIG-097).

## Status and progress

- **Where it runs**: workbench (built from `azerothcore-wotlk/modules/fl-eloxin`) and host since
  the first deploy (2026-07-13), both at `16dac3e`. The Vecna trio's DB bindings (MIG-097) are on
  the workbench since 2026-10-08 and on the host since HOST10 (2026-10-09); they removed three
  "not assigned" boot lines, nothing changes in game.
- **Evidence**: module test pass incl. Eloxin T2 (user-confirmed 2026-07-13); Vecna port T1
  (worldserver build, 2026-07-12). No work in this repo since 2026-07-12.
- **Done**: port to the current AzerothCore API (2026-07-09); Vecna port (2026-07-12).

## Next steps

1. Vecna: spawn him somewhere or leave him (queue row "W7 never-ported scripts"); his portal and
   add positions are hard-coded coordinates without a documented map.
2. Eloxin's phase-3 link spells 102003/102006/102007 are in neither the server `Spell.dbc` nor
   `acore_world.spell_dbc` (checked 2026-10-09): restore them or rework the mechanic (todo).
3. (suggestion) When next touched, drop the dead code (placeholder spell ids 12345 / 0, the
   unregistered `spell_vecna_aura`).

Open points in full: [todo.md](todo.md).

## Working here

- Branch `claude/<topic>-<sessionId>`, merge into `master` and push (project rule: no pull requests).
- C++ only: rebuild worldserver (`C:\wowstuff\dcore_bin`) and restart the workbench with
  `C:\wowstuff\ForgottenLand2.0\scripts\worldserver_restart.ps1`. Content changes (texts, loot,
  spawns) belong to the FL world DB, delivered through share-public `python_scripts/fl_content/sql/`
  with a MIG entry.
- New spell ids: check all three carriers (client and server `Spell.dbc`, `acore_world.spell_dbc`).
- Host-relevant changes need a MIG entry in share-public
  `docs/World of Warcraft/forgotten-land/15-host-migration-log.md`.
- Vault: `forgotten-land/06-fl-modules.md`, `12-server-todo.md` (W7 row), MIG-097 in
  `forgotten-land/15-host-migration-log.md`, `06-custom-ids.md`.
- Doc set: [INDEX.md](INDEX.md), CLAUDE.md, [data_structure.md](data_structure.md),
  [functions.md](functions.md), [log.md](log.md) (newest first), [todo.md](todo.md).
