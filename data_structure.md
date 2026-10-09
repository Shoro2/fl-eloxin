# Data structure

## Files

| Path | What |
|---|---|
| `src/boss_eloxin.cpp` | `boss_eloxin`, `eloxin_mushroom`, GameObject script `EloxinSummoner` |
| `src/boss_vecna.cpp` | `boss_vecna`, `vecna_imp`, `vecna_demon`; `spell_vecna_aura` is defined but never registered |
| `src/fl-eloxin_loader.cpp` | `Addfl_eloxinScripts()` -> `AddSC_BossEloxinScript()`, `AddSC_BossVecnaScript()` |
| `conf/fl-eloxin.conf.dist` | `FLEloxin.Enable = 1` (the code never reads it) |
| `data/sql/db-world/base/skeleton_module_acore_string.sql` | skeleton row `acore_string` 35410 "Hello World from Skeleton-Module!" |
| `data/sql/db-*/{base,updates}/.gitkeep`, `apps/`, `include.sh`, `pull_request_template.md` | AzerothCore skeleton leftovers (`apps/ci/ci-codestyle.sh` = the skeleton's codestyle check) |
| `.github/README.md`, `.github/README_ES.md`, `.github/ISSUE_TEMPLATE/` | skeleton template texts, not a module description |
| `.github/workflows/core-build.yml`, `core_codestyle.yml` | CI: AzerothCore's reusable module build + codestyle |
| `log.md` | change log |

## Rows the scripts depend on (FL world DB, not shipped here)

| Table | Rows |
|---|---|
| `creature_template` | 80067 (`boss_eloxin`), 69701-69705 (`eloxin_mushroom`), 9950 / 9953 / 9954 (Vecna trio), 9951 / 9952 (portals, no script) |
| `gameobject_template` + `gameobject` | 90001 `EloxinSummoner`, spawn guid 3501987 on map 727 |
| `creature_text` | 80067 groups 1-8 (8 is still "PH P3"); none for 9950 |
| `creature_loot_template` | 55555 (Eloxin) |
| `item_template` | essences 80011-80015 |
| server `Spell.dbc` | 550100-550111, 550500, 550501 |

The Vecna ScriptNames are written by share-public `python_scripts/fl_content/sql/42_fl_boot_loot_and_script_names.sql`.
No characters or auth tables.

## Config

`FLEloxin.Enable` (dist and workbench: 1) has no effect; the scripts always run.
