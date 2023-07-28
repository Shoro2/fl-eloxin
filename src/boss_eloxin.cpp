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
    SPELL_POISON_WAVE = 12345,
    SPELL_NEUROTOXIN = 12345,
    SPELL_POSION_CLAW = 12345,

    SPELL_POISON_BEAM = 63226,
    SPELL_FREEZE_ANIM = 16245,
    SPELL_SPAWN_MUSHROOMS = 12345,
    SPELL_MARK_OF_ELOXIN = 12345,

    SPELL_SPAWN_SACRIFICES = 12345,
    SPELL_EVOLVING_GROWTH = 12345,
    SPELL_SPAWN_GUARDIAN = 12345

};

enum Phases
{
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE

};

enum Events
{
    EVENT_SPELL_POISON_WAVE = 1,
    EVENT_SPELL_NEUROTOXIN = 2,
    EVENT_SPELL_POSION_CLAW = 3,

    EVENT_SPELL_POISON_BEAM = 4,
    EVENT_SPELL_POISON_BEAM_TICK = 15,
    EVENT_SPELL_SPAWN_MUSHROOMS = 5,
    EVENT_SPELL_MARK_OF_ELOXIN = 6,

    EVENT_SPELL_SPAWN_SACRIFICES = 7,
    EVENT_SPELL_EVOLVING_GROWTH = 8,
    EVENT_SPELL_SPAWN_GUARDIAN = 9,

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

    struct boss_eloxinAI : public BossAI
    {
        boss_eloxinAI(Creature* creature) : BossAI(creature, 0) { }
        void Initialize()
        {
            events.ScheduleEvent(EVENT_SPELL_POISON_BEAM, 30000);
        }

        void SetPhase(uint8 ph)
        {
            events.Reset();
            Phase = ph;
            switch (ph)
            {
            case PHASE_ONE:
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM, 30000);
                //events.ScheduleEvent(EVENT_SPELL_NEUROTOXIN, urand(5000, 10000));
                //events.ScheduleEvent(EVENT_SPELL_POSION_CLAW, urand(15000, 20000));
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
            Initialize();
            DarkGlareTick = 0;
            DarkGlareAngle = 0;
            ClockWise = false;
            ClockWise = RAND(true, false);
            events.Reset();
            BossAI::Reset();
        }

        void JustEngagedWith(Unit* who) override
        {
            Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            BossAI::JustEngagedWith(who);
            Initialize();
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
            case EVENT_SPELL_POISON_BEAM:

                sWorld->SendWorldText(LANG_EVENTMESSAGE, "Start spinning");
                DoCast(me, SPELL_FREEZE_ANIM, true);
                me->StopMoving();

                ClockWise = RAND(true, false);
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true))
                {
                    DarkGlareAngle = me->GetAngle(target); //keep as the location dark glare will be at
                }
                me->SetOrientation(DarkGlareAngle);
                me->SetFacingTo(DarkGlareAngle);
                DarkGlareTick = 0;

                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_TICK, 1000);
                events.RepeatEvent(60000);
                break;

            case EVENT_SPELL_POISON_BEAM_TICK:
                sWorld->SendWorldText(LANG_EVENTMESSAGE, "spin tick");
                angle = ClockWise ? DarkGlareAngle + DarkGlareTick * float(M_PI) / 35 : DarkGlareAngle - DarkGlareTick * float(M_PI) / 35;
                me->SetFacingTo(angle);
                me->SetOrientation(angle);

                DoCastSelf(SPELL_POISON_BEAM);

                ++DarkGlareTick;

                if (DarkGlareTick >= 35) {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAurasDueToSpell(SPELL_FREEZE_ANIM);
                    me->InterruptNonMeleeSpells(false);
                    sWorld->SendWorldText(LANG_EVENTMESSAGE, "spin done");
                }
                else {
                    events.RepeatEvent(1000);
                }
                break;

            case EVENT_START_PHASE_2:
                if (me->HasUnitFlag(UNIT_FLAG_DISARMED))
                {
                    events.RepeatEvent(5000);
                }
                else
                {
                    events.RepeatEvent(urand(18000, 21000));
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
        uint32 DarkGlareTick;
        float DarkGlareAngle;
        bool ClockWise;
        float angle;
    };
};

void AddSC_BossEloxinScript()
{
    new boss_eloxin();
    new FLEloxinPlayer();
}
