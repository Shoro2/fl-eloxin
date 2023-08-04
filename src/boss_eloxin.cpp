#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"

enum Texts
{
    SAY_DEATH = 0,
    SAY_ENGAGE = 1,
    SAY_ONKILL = 2,
    SAY_EVADE = 3
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
    SPELL_MARK_OF_ELOXIN = 12345,

    SPELL_SPAWN_SACRIFICES = 12345,
    SPELL_EVOLVING_GROWTH = 12345,
    SPELL_SPAWN_GUARDIAN = 12345,

    SPELL_POISON_BOLT_VOLLEY = 550105
};

enum Phases
{
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE

};

enum Events
{
    EVENT_SPELL_POISON_NOVA = 1,
    EVENT_SPELL_NEUROTOXIN = 2,
    EVENT_SPELL_SEPSIS = 3,

    EVENT_SPELL_POISON_BEAM = 4,
    EVENT_SPELL_POISON_BEAM_TICK = 15,
    EVENT_SPELL_POISON_BEAM_CAST = 16,
    EVENT_SPELL_SPAWN_MUSHROOMS = 5,
    EVENT_SPELL_MARK_OF_ELOXIN = 6,

    EVENT_SPELL_SPAWN_SACRIFICES = 7,
    EVENT_SPELL_EVOLVING_GROWTH = 8,
    EVENT_SPELL_SPAWN_GUARDIAN = 9,

    EVENT_SPELL_POISON_BOLT_VOLLEY = 17,
    EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED = 18,

    EVENT_START_PULL = 10,
    EVENT_START_PHASE_2 = 11,
    EVENT_START_PHASE_3 = 12,
    EVENT_END_WIPE = 13,
    EVENT_END_DEATH = 14

    
};

class FLEloxinPlayer : public PlayerScript
{
public:
    FLEloxinPlayer() : PlayerScript("FLEloxinPlayer") { }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("FLEloxin.Enable", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Hello World from Eloxin-Module!");
        }
    }
};

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
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, 60000);
                break;
            case PHASE_TWO:
                events.ScheduleEvent(EVENT_START_PHASE_2, 0);
                
                break;
            case PHASE_THREE:
                events.ScheduleEvent(EVENT_START_PHASE_3, 5000);
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
        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            ScriptedAI::JustEngagedWith(who);
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
            if (me->GetDistance(eloxin_target->GetPosition()) > 15 && !castedPBV) {
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
                PB_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                PoisonBeamAngle = me->GetAngle(PB_target);
                me->SetOrientation(PoisonBeamAngle);
                me->SetFacingTo(PoisonBeamAngle);
                me->StopMoving();
                DoCastSelf(SPELL_POISON_BEAM_CAST);
                me->SetTarget(ObjectGuid::Empty);
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM, 5000);
                break;

            case EVENT_SPELL_POISON_BEAM:
                ClockWise = RAND(true, false);
                PoisonBeamTick = 0;
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_TICK, 100);
                break;

            case EVENT_SPELL_POISON_BEAM_TICK:
                angle = ClockWise ? PoisonBeamAngle + PoisonBeamTick * float(M_PI) / 35 : PoisonBeamAngle - PoisonBeamTick * float(M_PI) / 35;
                me->SetFacingTo(angle);
                me->SetOrientation(angle);

                DoCastVictim(SPELL_POISON_BEAM);

                ++PoisonBeamTick;

                if (PoisonBeamTick >= 60) {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->InterruptNonMeleeSpells(false);
                    events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, 10000);
                    me->SetTarget(PB_target->GetGUID());
                    isSpinning = false;
                }
                else {
                    events.RepeatEvent(100);
                }
                break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            // Implement boss death behavior
        }

    private:
        uint8 Phase;
        uint32 PoisonBeamTick;
        float PoisonBeamAngle;
        bool castedPBV = false;
        bool ClockWise, isSpinning;
        float angle;
        Unit* PB_target;
        Unit* eloxin_target;
        Unit* Sepsis_target;
        Unit* NT_target;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_eloxinAI(creature);
    }
};

void AddSC_BossEloxinScript()
{
    new boss_eloxin();
    new FLEloxinPlayer();
}
