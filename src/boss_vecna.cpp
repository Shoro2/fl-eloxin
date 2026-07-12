#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"

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

enum Adds
{
    NPC_VECNA_IMP = 9953,
    NPC_VECNA_DEMON = 9954,
    NPC_VECNA_PORTAL_1 = 9951,
    NPC_VECNA_PORTAL_2 = 9952,
};

enum Phases
{
    PHASE_ENGAGE,
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE,
    PHASE_END,

};

enum Events
{
    EVENT_PORTAL_ONE = 1,
    EVENT_PORTAL_TWO = 2,

    EVENT_SPELL_POOL = 3,
    EVENT_SPELL_DEMON_PULSE = 4,

    EVENT_SPAWN_IMP = 5,
    EVENT_SPAWN_DEMON = 6,

    EVENT_AURA_START = 7,

};

enum Spells
{
    SPELL_BOSS_AURA = 550501,
    SPELL_IMP_EXPLODE = 0,
    SPELL_DEMON_EXPLODE = 550500,
    SPELL_POOL = 0,
    SPELL_DEMON_AURA_PULSE = 0,

};

class boss_vecna : public CreatureScript
{
public:
    boss_vecna() : CreatureScript("boss_vecna") { }

    struct boss_vecnaAI : public ScriptedAI
    {
        boss_vecnaAI(Creature* creature) : ScriptedAI(creature),
            Phase(PHASE_ENGAGE), pool_target(nullptr), portalOne(nullptr),
            portalTwo(nullptr)
        {
        }

        void SetPhase(uint8 ph)
        {
            events.Reset();
            Phase = ph;
            switch (ph)
            {
            case PHASE_ONE:
                events.ScheduleEvent(EVENT_PORTAL_ONE, Milliseconds(1000));
                Talk(SAY_ENGAGE);
                break;
            case PHASE_TWO:
                events.ScheduleEvent(EVENT_PORTAL_TWO, Milliseconds(1000));
                Talk(SAY_P2);
                break;
            case PHASE_THREE:
                events.ScheduleEvent(EVENT_SPELL_POOL, Milliseconds(25000));
                Talk(SAY_P3);
                break;
            }
        }

        void Reset() override
        {
            events.Reset();
            Phase = PHASE_ENGAGE;
            pool_target = nullptr;
            portalOne = nullptr;
            portalTwo = nullptr;
            ScriptedAI::Reset();
        }

        void RemoveSummons() {
            std::list<Creature*> impsInRange;
            std::list<Creature*> demonsInRange;

            me->GetCreaturesWithEntryInRange(
                impsInRange, 200, NPC_VECNA_IMP);
            me->GetCreaturesWithEntryInRange(
                demonsInRange, 200, NPC_VECNA_DEMON);

            if (portalOne && portalOne->IsAlive()) {
                portalOne->DespawnOrUnsummon();
            }

            if (portalTwo && portalTwo->IsAlive()) {
                portalTwo->DespawnOrUnsummon();
            }

            for (Creature* creature : impsInRange) {
                creature->DespawnOrUnsummon();
            }
            for (Creature* creature : demonsInRange) {
                creature->DespawnOrUnsummon();
            }

        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            ScriptedAI::JustEngagedWith(who);
            events.ScheduleEvent(EVENT_AURA_START, Milliseconds(1000));
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType,
            SpellSchoolMask) override
        {
            if (me->HealthBelowPctDamaged(95, damage) && Phase == PHASE_ENGAGE)
            {
                SetPhase(PHASE_ONE);
            }
            else if (me->HealthBelowPctDamaged(85, damage) &&
                Phase == PHASE_ONE)
            {
                SetPhase(PHASE_TWO);
            }
            else if (me->HealthBelowPctDamaged(60, damage) &&
                Phase == PHASE_TWO)
            {
                SetPhase(PHASE_THREE);
            }
            else if (me->HealthBelowPctDamaged(30, damage) &&
                Phase == PHASE_THREE)
            {
                SetPhase(PHASE_END);
            }
            /*
            else if (me->HealthBelowPctDamaged(40, damage) &&
                Phase == PHASE_TWO)
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

            switch (events.ExecuteEvent())
            {

            case EVENT_PORTAL_ONE:
                //spawn portal
                portalOne = me->SummonCreature(NPC_VECNA_PORTAL_1, -174.97644,
                    -813.807129, 41.620617, 2.28,
                    TEMPSUMMON_MANUAL_DESPAWN, 100);
                events.ScheduleEvent(EVENT_SPAWN_IMP, Milliseconds(2000));
                break;
            case EVENT_PORTAL_TWO:
                //spawn portal
                portalTwo = me->SummonCreature(NPC_VECNA_PORTAL_2, -174.97644,
                    -813.807129, 41.620617, 2.28,
                    TEMPSUMMON_MANUAL_DESPAWN, 100);
                events.ScheduleEvent(EVENT_SPAWN_DEMON, Milliseconds(2000));
                break;

            case EVENT_SPAWN_IMP:
                if (portalOne && portalOne->IsAlive()) {
                    me->SummonCreature(NPC_VECNA_IMP, -174.97644,
                        -813.807129, 41.620617, 2.28,
                        TEMPSUMMON_CORPSE_DESPAWN);
                }
                events.Repeat(Milliseconds(10000));
                break;

            case EVENT_SPAWN_DEMON:
                if (portalTwo && portalTwo->IsAlive()) {
                    me->SummonCreature(NPC_VECNA_DEMON, -174.97644,
                        -813.807129, 41.620617, 2.28,
                        TEMPSUMMON_CORPSE_DESPAWN);
                }
                events.Repeat(Milliseconds(35000));
                break;

            case EVENT_SPELL_POOL:
                pool_target = SelectTarget(
                    SelectTargetMethod::Random, 0, 0.0f, true);
                if (pool_target && SPELL_POOL)
                {
                    DoCast(pool_target, SPELL_POOL, true);
                }
                events.Repeat(Milliseconds(urand(20000, 25000)));
                break;

            case EVENT_AURA_START:
                DoCastAOE(SPELL_BOSS_AURA);
                events.Repeat(Milliseconds(2000));
                break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            // Implement boss death behavior
            Talk(SAY_DEATH);
            RemoveSummons();
            //events.ScheduleEvent(EVENT_DESPAWN, 10000);
        }

    private:
        uint8 Phase;
        Unit* pool_target;
        Creature* portalOne;
        Creature* portalTwo;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_vecnaAI(creature);
    }
};

class vecna_imp : public CreatureScript
{
public:
    vecna_imp() : CreatureScript("vecna_imp") { }

    struct vecna_impAI : public ScriptedAI
    {
        vecna_impAI(Creature* creature) : ScriptedAI(creature) {
        }




        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

        }

        void JustDied(Unit* /*pKiller*/) override {

                DoCastAOE(SPELL_IMP_EXPLODE, true);
                me->DespawnOrUnsummon();

        }

    private:

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new vecna_impAI(creature);
    }
};

class vecna_demon : public CreatureScript
{
public:
    vecna_demon() : CreatureScript("vecna_demon") { }

    struct vecna_demonAI : public ScriptedAI
    {
        vecna_demonAI(Creature* creature) : ScriptedAI(creature) {
        }




        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);
            switch (events.ExecuteEvent())
            {
            case EVENT_SPELL_DEMON_PULSE:
                DoCast(SPELL_DEMON_AURA_PULSE);
                events.Repeat(Milliseconds(1000));
                break;
            
            }
        }

        void JustDied(Unit* /*pKiller*/) override {

            DoCastAOE(SPELL_DEMON_EXPLODE, true);
            me->DespawnOrUnsummon();

        }

    private:

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new vecna_demonAI(creature);
    }
};


class spell_vecna_aura : public SpellScript
{
    PrepareSpellScript(spell_vecna_aura);

    void HandleScriptEffect(SpellEffIndex  /*effIndex*/)
    {
        
        if (Unit* target = GetHitPlayer()) {
            Unit* caster = GetCaster();
            if (Aura* aur = target->GetAura(GetSpellInfo()->Id))
                if (aur->GetStackAmount() >= 100)
                    caster->Kill(caster, target);
        }
            
    }


};

void AddSC_BossVecnaScript()
{
    new boss_vecna();
    new vecna_imp();
    new vecna_demon();
}
