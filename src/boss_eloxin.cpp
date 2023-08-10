#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ScriptedGossip.h"

//entry: 80067

enum Texts
{
    SAY_DEATH = 1,
    SAY_ENGAGE = 2,
    SAY_ONKILL = 3,
    SAY_EVADE = 4,
    SAY_LEFT = 5,
    SAY_RIGHT = 6,
    SAY_P2 = 7,
    SAY_P3 = 8
};

enum Spells
{
    SPELL_POISON_NOVA = 550102,
    SPELL_NEUROTOXIN = 550103,
    SPELL_SEPSIS = 550104,

    SPELL_POISON_BEAM = 550100,
    SPELL_POISON_BEAM_CAST = 550101,
    SPELL_FREEZE_ANIM = 16245,
    SPELL_SPAWN_MUSHROOMS = 12345,
    SPELL_MARK_OF_ELOXIN = 550106,

    SPELL_SPAWN_SACRIFICES = 12345,
    SPELL_EVOLVING_GROWTH = 12345,
    SPELL_SPAWN_GUARDIAN = 12345,

    SPELL_POISON_BOLT_VOLLEY = 550105,

    SPELL_MUSHROOM_RED_DEATH = 550107,
    SPELL_MUSHROOM_BLUE_DEATH = 550108,
    SPELL_MUSHROOM_GREEN_DEATH = 550109,
    SPELL_MUSHROOM_BLUE_PULSE = 550110,

    SPELL_AURA_SYMBIOSIS = 550111
};

enum Phases
{
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE

};

enum Mushrooms
{
    MUSHROOM_GREEN = 69702,
    MUSHROOM_RED = 69703,
    MUSHROOM_BLUE = 69704,
    
};

enum Events
{
    EVENT_SPELL_POISON_NOVA = 1,
    EVENT_SPELL_NEUROTOXIN = 2,
    EVENT_SPELL_SEPSIS = 3,

    EVENT_SPELL_POISON_BEAM = 4,
    
    EVENT_SPELL_SPAWN_MUSHROOMS = 5,
    EVENT_SPELL_MARK_OF_ELOXIN = 6,

    EVENT_SPELL_SPAWN_SACRIFICES = 7,
    EVENT_SPELL_EVOLVING_GROWTH = 8,
    EVENT_SPELL_SPAWN_GUARDIAN = 9,

    EVENT_START_PULL = 10,
    EVENT_START_PHASE_2 = 11,
    EVENT_START_PHASE_3 = 12,
    EVENT_END_WIPE = 13,
    EVENT_END_DEATH = 14,

    EVENT_SPELL_POISON_BEAM_TICK = 15,
    EVENT_SPELL_POISON_BEAM_CAST = 16,

    EVENT_SPELL_POISON_BOLT_VOLLEY = 17,
    EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED = 18,

    EVENT_SPELL_MUSHROOM_BLUE_PULSE = 19,
    EVENT_SPELL_MUSHROOM_GREEN_PULSE = 20,

    EVENT_AURA_REFRESH_TICK = 21,

    
};

uint32 auraStacks = 0;

class boss_eloxin : public CreatureScript
{
public:
    boss_eloxin() : CreatureScript("boss_eloxin") { }
    
    struct boss_eloxinAI : public ScriptedAI
    {
        boss_eloxinAI(Creature* creature) : ScriptedAI(creature) {
            
        }

        void SetPhase(uint8 ph)
        {
            events.Reset();
            Phase = ph;
            switch (ph)
            {
            case PHASE_ONE:
                events.ScheduleEvent(EVENT_SPELL_POISON_NOVA, 35000);
                events.ScheduleEvent(EVENT_SPELL_NEUROTOXIN, urand(5000, 10000));
                events.ScheduleEvent(EVENT_SPELL_SEPSIS, 11000);
                Talk(SAY_ENGAGE);
                break;
            case PHASE_TWO:
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, 50000);
                events.ScheduleEvent(EVENT_SPELL_SPAWN_MUSHROOMS, 10000);
                events.ScheduleEvent(EVENT_SPELL_MARK_OF_ELOXIN, 25000);
                events.ScheduleEvent(EVENT_AURA_REFRESH_TICK, 10000);
                events.ScheduleEvent(EVENT_SPELL_SEPSIS, 11000);
                Talk(SAY_P2);
                break;
            case PHASE_THREE:
                events.ScheduleEvent(EVENT_START_PHASE_3, 5000);
                Talk(SAY_P3);
                break;
            }
        }

        void Reset() override
        {
            // Implement boss reset behavior
            PoisonBeamTick = 0;
            PoisonBeamAngle = 0;
            ClockWise = false;
            events.Reset();
            ScriptedAI::Reset();
            auraStacks = 0;
            RemoveMushrooms();
            me->RemoveAura(SPELL_AURA_SYMBIOSIS);
        }

        void RemoveMushrooms() {
            std::list<Creature*> greenShroomsInRange;
            std::list<Creature*> redShroomsInRange;
            std::list<Creature*> blueShroomsInRange;
            me->GetCreaturesWithEntryInRange(greenShroomsInRange, 200, MUSHROOM_GREEN);
            me->GetCreaturesWithEntryInRange(redShroomsInRange, 200, MUSHROOM_RED);
            me->GetCreaturesWithEntryInRange(blueShroomsInRange, 200, MUSHROOM_BLUE);

            for (Creature* creature : greenShroomsInRange) {
                creature->DespawnOrUnsummon();
            }
            for (Creature* creature : redShroomsInRange) {
                creature->DespawnOrUnsummon();
            }
            for (Creature* creature : blueShroomsInRange) {
                creature->DespawnOrUnsummon();
            }

            me->SetAuraStack(SPELL_AURA_SYMBIOSIS, me, 0);
            
                       
        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            ScriptedAI::JustEngagedWith(who);
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override
        {
            if (me->HealthBelowPctDamaged(50, damage) && Phase == PHASE_ONE)
            {
                SetPhase(PHASE_TWO);
            }
            /*
            else if (me->HealthBelowPctDamaged(40, damage) && Phase == PHASE_TWO)
            {
                SetPhase(PHASE_THREE);
            }
            */
        }

        void UpdateAI(uint32 diff) override
        {
            // Implement boss update behavior (abilities, movement, etc.)
            if (!UpdateVictim() || !CheckInRoom())
            {
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
            {
                return;
            }

            DoMeleeAttackIfReady();

            eloxin_target = me->SelectNearestTarget();
            if (me->GetDistance(eloxin_target->GetPosition()) > 25 && !castedPBV) {
                events.ScheduleEvent(EVENT_SPELL_POISON_BOLT_VOLLEY, 0);
                events.ScheduleEvent(EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED, 1000);
                castedPBV = true;
            }

            switch (events.ExecuteEvent())
            {
            case EVENT_SPELL_POISON_NOVA:
                DoCast(SPELL_POISON_NOVA);
                events.RepeatEvent(35000);
                break;
            case EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED:
                castedPBV = false;
                break;

            case EVENT_SPELL_POISON_BOLT_VOLLEY:
                DoCast(SPELL_POISON_BOLT_VOLLEY);
                break;

            case EVENT_SPELL_SEPSIS:
                Sepsis_target = SelectTarget(SelectTargetMethod::MaxThreat, 0, 0.0f, true);
                DoCast(Sepsis_target, SPELL_SEPSIS, true);
                events.RepeatEvent(11000);
                break;

            case EVENT_SPELL_NEUROTOXIN:
                NT_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                DoCast(NT_target, SPELL_NEUROTOXIN, true);
                events.RepeatEvent(urand(5000, 10000));
                break;

            case EVENT_SPELL_POISON_BEAM_CAST:
                isSpinning = true;
                me->InterruptNonMeleeSpells(true);
                PB_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                PoisonBeamAngle = me->GetAngle(PB_target);
                me->SetOrientation(PoisonBeamAngle);
                me->SetFacingTo(PoisonBeamAngle);
                me->StopMoving();
                //DoCast(me, SPELL_FREEZE_ANIM, true);
                DoCastSelf(SPELL_POISON_BEAM_CAST, false);
                me->SetTarget(ObjectGuid::Empty);
                ClockWise = RAND(true, false);
                if (ClockWise) {
                    Talk(SAY_LEFT);
                }
                else {
                    Talk(SAY_RIGHT);
                }
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM, 5100);
                break;

            case EVENT_SPELL_POISON_BEAM:
                
                PoisonBeamTick = 0;
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_TICK, 100);
                break;

            case EVENT_SPELL_POISON_BEAM_TICK:
                angle = ClockWise ? PoisonBeamAngle + PoisonBeamTick * float(M_PI) / 35 : PoisonBeamAngle - PoisonBeamTick * float(M_PI) / 35;
                me->SetFacingTo(angle);
                me->SetOrientation(angle);
                DoCastSelf(SPELL_POISON_BEAM, false);

                ++PoisonBeamTick;

                if (PoisonBeamTick >= 40) {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->InterruptNonMeleeSpells(false);
                    events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, 50000);
                    me->SetTarget(PB_target->GetGUID());
                    //me->RemoveAurasDueToSpell(SPELL_FREEZE_ANIM);
                    isSpinning = false;
                }
                else {
                    events.RepeatEvent(300);
                }
                break;
            case EVENT_SPELL_SPAWN_MUSHROOMS:
                Shroom_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                randomInt = RAND(1, 2, 3);
                if (randomInt == 1) {
                    spawnedShroom = me->SummonCreature(MUSHROOM_RED, Shroom_target->GetPositionX(), Shroom_target->GetPositionY(), Shroom_target->GetPositionZ(), Shroom_target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100);
                }
                else if (randomInt == 2) {
                    spawnedShroom = me->SummonCreature(MUSHROOM_BLUE, Shroom_target->GetPositionX(), Shroom_target->GetPositionY(), Shroom_target->GetPositionZ(), Shroom_target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100);
                }
                else if (randomInt == 3) {
                    spawnedShroom = me->SummonCreature(MUSHROOM_GREEN, Shroom_target->GetPositionX(), Shroom_target->GetPositionY(), Shroom_target->GetPositionZ(), Shroom_target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100);
                }
                spawnedShroom->CombatStart(Shroom_target, true);
                spawnedShroom->SetTarget();
                events.RepeatEvent(10000);
                auraStacks++;
                break;

            case EVENT_SPELL_MARK_OF_ELOXIN:
                MOE_target = SelectTarget(SelectTargetMethod::MaxDistance, 0, 0.0f, true);
                DoCast(MOE_target, SPELL_MARK_OF_ELOXIN, true);
                events.RepeatEvent(25000);
                break;

            case EVENT_AURA_REFRESH_TICK:
                if (auraStacks > 0) {
                    me->AddAura(SPELL_AURA_SYMBIOSIS, me);
                    me->SetAuraStack(SPELL_AURA_SYMBIOSIS, me, auraStacks);
                    
                }
                else {
                    me->RemoveAura(SPELL_AURA_SYMBIOSIS);
                }
                events.RepeatEvent(1000);
                break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            // Implement boss death behavior
            Talk(SAY_DEATH);
            RemoveMushrooms();
        }

    private:
        uint8 Phase, randomInt;
        uint32 PoisonBeamTick;
        float PoisonBeamAngle;
        bool castedPBV = false;
        bool ClockWise, isSpinning;
        float angle;
        Unit* PB_target;
        Unit* eloxin_target;
        Unit* Sepsis_target;
        Unit* NT_target;
        Unit* Shroom_target;
        Unit* MOE_target;
        Unit* spawnedShroom;
    };



    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_eloxinAI(creature);
    }
};

class eloxin_mushroom : public CreatureScript
{
public:
    eloxin_mushroom() : CreatureScript("eloxin_mushroom") { }

    struct eloxin_mushroomAI : public ScriptedAI
    {
        eloxin_mushroomAI(Creature* creature) : ScriptedAI(creature) {
        }

        void JustEngagedWith(Unit* /*who*/) override {
            if (me->GetEntry() == MUSHROOM_GREEN) {
                me->SetHealth(1);
                events.ScheduleEvent(EVENT_SPELL_MUSHROOM_GREEN_PULSE, 500);
            }

            if (me->GetEntry() == MUSHROOM_BLUE) {
                events.ScheduleEvent(EVENT_SPELL_MUSHROOM_BLUE_PULSE, 1000);
            }

        }


        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_SPELL_MUSHROOM_BLUE_PULSE:
                DoCast(SPELL_MUSHROOM_BLUE_PULSE);
                events.RepeatEvent(1000);
                break;
            case EVENT_SPELL_MUSHROOM_GREEN_PULSE:
                if (me->IsFullHealth() ) {
                    DoCast(SPELL_MUSHROOM_GREEN_DEATH);
                    me->DespawnOrUnsummon();
                }
                else events.RepeatEvent(500);
                break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override {
            if (me->GetEntry() == MUSHROOM_RED) {
                DoCastAOE(SPELL_MUSHROOM_RED_DEATH, true);
                me->DespawnOrUnsummon();
            }
            else if (me->GetEntry() == MUSHROOM_BLUE) {
                DoCastAOE(SPELL_MUSHROOM_BLUE_DEATH, true);
                me->DespawnOrUnsummon();
            }
            auraStacks--;
        }

    private:

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new eloxin_mushroomAI(creature);
    }
};


class EloxinSummoner : public GameObjectScript
{
public:
    EloxinSummoner() : GameObjectScript("EloxinSummoner") { }



    struct EloxinSummonerAI : public GameObjectAI
    {
        EloxinSummonerAI(GameObject* go) : GameObjectAI(go) {}

        virtual void SummonedCreatureDespawn(Creature* /*summon*/) {
            me->RemoveGameObjectFlag(GO_FLAG_NOT_SELECTABLE);
        }


        bool GossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
        {
            uint32 magicalessenceAir, magicalessenceFire, magicalessenceEarth, magicalessenceShaddow;
            TempSummon* myBoss;
            Seconds respawnTimer{};

            player->PlayerTalkClass->SendCloseGossip();

            Creature* lastSpawn = ObjectAccessor::GetCreature(*me, _creatureGuid);
            if (lastSpawn && lastSpawn->IsAlive())
            {
                // We already summoned something recently, return.
                CloseGossipMenuFor(player);
                return true;
            }
            else
            {
                _creatureGuid.Clear();
            }

            respawnTimer = 60s;

            switch (action) {
            case 0:
                //flame boss
                magicalessenceAir = player->GetItemCount(80011);
                magicalessenceEarth = player->GetItemCount(80013);
                magicalessenceShaddow = player->GetItemCount(80015);
                magicalessenceFire = player->GetItemCount(80012);

                if (magicalessenceAir >= 1 && magicalessenceEarth >= 1 && magicalessenceShaddow >= 1 && magicalessenceFire >= 1) {
                    //summon boss
                    player->DestroyItemCount(80011, 1, true);
                    player->DestroyItemCount(80013, 1, true);
                    player->DestroyItemCount(80015, 1, true);
                    player->DestroyItemCount(80012, 1, true);
                    myBoss = me->SummonCreature(80067, 13421.22, 11432.24, -155.05, 3.9941, TEMPSUMMON_MANUAL_DESPAWN);
                    myBoss->SetCorpseDelay(5 * MINUTE);
                    _creatureGuid = myBoss->GetGUID();
                    CloseGossipMenuFor(player);
                    me->SetGameObjectFlag(GO_FLAG_NOT_SELECTABLE);
                    me->DespawnOrUnsummon(5000ms, respawnTimer); // Despawn in 5 Seconds for respawnTimer value



                }
                else {
                    ChatHandler(player->GetSession()).SendSysMessage("You dont have all the Magical Essence!");
                }
                break;
            case 1:
                //good bye
                CloseGossipMenuFor(player);
                break;
            default:
                //good bye
                CloseGossipMenuFor(player);
                break;
            }


            return true;
        }

    private:
        ObjectGuid _creatureGuid;
    };

    GameObjectAI* GetAI(GameObject* go) const
    {
        return new EloxinSummonerAI(go);
    }
};



void AddSC_BossEloxinScript()
{
    new boss_eloxin();
    new eloxin_mushroom();
    new EloxinSummoner();
}
