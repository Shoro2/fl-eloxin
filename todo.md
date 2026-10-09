# Todo

- (medium) Vecna (9950): spawn him somewhere or leave him (share-public queue row "W7 never-ported
  scripts"). The ScriptNames are bound since MIG-097, but nothing spawns him, and his portal/add
  positions are hard-coded coordinates without a documented map.
- (medium) Eloxin's phase-3 link spells 102003, 102006 and 102007 are in neither the server
  `Spell.dbc` nor `acore_world.spell_dbc` (workbench, 2026-10-09), so the link casts fail. Find
  the old FL spells or rework the mechanic.
- (low) Text group 8 of 80067 (phase 3) is still the placeholder "PH P3" (FL world content).
- (low) (suggestion) Drop the dead code when next touched: placeholder spell ids 12345 and 0,
  the unregistered `spell_vecna_aura`, and `FLEloxin.Enable`, which nothing reads.
