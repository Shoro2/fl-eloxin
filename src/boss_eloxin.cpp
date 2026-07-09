#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ScriptedGossip.h"
#include "Log.h"

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

    SPELL_AURA_SYMBIOSIS = 550111,

    SPELL_LINK_PLAYERS = 102003,
    SPELL_LINK_PLAYERS_DAMAGE = 550106,
    SPELL_LINK_PLAYERS_VISUAL = 102006,
    SPELL_AURA_DMG_LINK = 102007,
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

    EVENT_LINK_PLAYERS = 22,
    EVENT_LINK_PLAYERS_DAMAGE = 23,
    EVENT_LINK_PLAYERS_DONE = 24,

    EVENT_DESPAWN = 25,


};

uint32 auraStacks = 0;

class boss_eloxin : public CreatureScript
{
public:
    boss_eloxin() : CreatureScript("boss_eloxin") {}

    struct boss_eloxinAI : public ScriptedAI
    {
        boss_eloxinAI(Creature* creature) : ScriptedAI(creature) {

        }


        void SetPhase(uint8 ph)
        {
            Phase = ph;
            switch (ph)
            {
            case PHASE_ONE: {
                events.ScheduleEvent(EVENT_SPELL_POISON_NOVA, Milliseconds(35000));
                events.ScheduleEvent(EVENT_SPELL_NEUROTOXIN, Milliseconds(urand(5000, 10000)));
                events.ScheduleEvent(EVENT_SPELL_SEPSIS, Milliseconds(11000));
                break;
            }
            case PHASE_TWO: {
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, Milliseconds(50000));
                events.ScheduleEvent(EVENT_SPELL_SPAWN_MUSHROOMS, Milliseconds(10000));
                Talk(SAY_P2);
                break;
            }
            case PHASE_THREE: {
                events.ScheduleEvent(EVENT_START_PHASE_3, Milliseconds(5000));
                events.ScheduleEvent(EVENT_LINK_PLAYERS, Milliseconds(2000));
                Talk(SAY_P3);
                break;
            }
            }
        }

        void Reset() override
        {
            //// Implement boss reset behavior
            PoisonBeamTick = 0;
            PoisonBeamAngle = 0;
            ClockWise = false;
            LOG_DEBUG("module", "Resetting Events");
            events.Reset();
            LOG_DEBUG("module", "Resetting AI");
            ScriptedAI::Reset();
            auraStacks = 0;
            RemoveMushrooms();
            target_link1 = nullptr;
            target_link2 = nullptr;
            playersInRange.clear();
            if (me->HasAura(SPELL_AURA_SYMBIOSIS)) me->RemoveAura(SPELL_AURA_SYMBIOSIS);

        }

        void RemoveMushrooms() {
            std::list<Creature*> greenShroomsInRange;
            std::list<Creature*> redShroomsInRange;
            std::list<Creature*> blueShroomsInRange;
            me->GetCreaturesWithEntryInRange(greenShroomsInRange, 200, MUSHROOM_GREEN);
            me->GetCreaturesWithEntryInRange(redShroomsInRange, 200, MUSHROOM_RED);
            me->GetCreaturesWithEntryInRange(blueShroomsInRange, 200, MUSHROOM_BLUE);
            LOG_DEBUG("module", "Got shrooms");
            for (Creature* creature : greenShroomsInRange) {
                if(creature) creature->DespawnOrUnsummon();
                LOG_DEBUG("module", "green");
            }
            for (Creature* creature : redShroomsInRange) {
                if (creature) creature->DespawnOrUnsummon();
                LOG_DEBUG("module", "red");
            }
            for (Creature* creature : blueShroomsInRange) {
                if (creature) creature->DespawnOrUnsummon();
                LOG_DEBUG("module", "blue");
            }
            LOG_DEBUG("module", "Removed shrooms");
        }

        void JustEngagedWith(Unit* who) override
        {
            //Talk(SAY_ENGAGE);
            SetPhase(PHASE_ONE);
            ScriptedAI::JustEngagedWith(who);
        }

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override
        {
            if (me->HealthBelowPctDamaged(66, damage) && Phase == PHASE_ONE)
            {
                SetPhase(PHASE_TWO);
            }
            else if (me->HealthBelowPctDamaged(33, damage) && Phase == PHASE_TWO)
            {
                SetPhase(PHASE_THREE);
            }

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
            if (eloxin_target && me->GetDistance(eloxin_target->GetPosition()) > 25 && !castedPBV)
            {
                events.ScheduleEvent(EVENT_SPELL_POISON_BOLT_VOLLEY, Milliseconds(0));
                events.ScheduleEvent(EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED, Milliseconds(1000));
                castedPBV = true;
            }


            switch (events.ExecuteEvent())
            {
            case EVENT_DESPAWN: {
                me->DespawnOrUnsummon();
                break;
            }
            case EVENT_SPELL_POISON_NOVA: {
                DoCast(SPELL_POISON_NOVA);
                events.Repeat(Milliseconds(35000));
                break;
            }
            case EVENT_SPELL_POISON_BOLT_VOLLEY_CASTED: {
                castedPBV = false;
                break;
            }
            case EVENT_SPELL_POISON_BOLT_VOLLEY: {
                DoCast(SPELL_POISON_BOLT_VOLLEY);
                break;
            }
            case EVENT_SPELL_SEPSIS: {
                Sepsis_target = SelectTarget(SelectTargetMethod::MaxThreat, 0, 0.0f, true);
                DoCast(Sepsis_target, SPELL_SEPSIS, true);
                events.Repeat(Milliseconds(10000));
                break;
            }
            case EVENT_SPELL_NEUROTOXIN: {
                NT_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                DoCast(NT_target, SPELL_NEUROTOXIN, true);
                NT_target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                DoCast(NT_target, SPELL_NEUROTOXIN, true);
                events.Repeat(Milliseconds(urand(3000, 6000)));
                break;
            }
            case EVENT_SPELL_POISON_BEAM_CAST: {
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
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM, Milliseconds(5100));
                break;
            }
            case EVENT_SPELL_POISON_BEAM: {

                PoisonBeamTick = 0;
                events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_TICK, Milliseconds(100));
                break;
            }
            case EVENT_SPELL_POISON_BEAM_TICK: {
                angle = ClockWise ? PoisonBeamAngle + PoisonBeamTick * float(M_PI) / 35 : PoisonBeamAngle - PoisonBeamTick * float(M_PI) / 35;
                me->SetFacingTo(angle);
                me->SetOrientation(angle);
                events.CancelEvent(EVENT_SPELL_POISON_NOVA);
                events.CancelEvent(EVENT_SPELL_NEUROTOXIN);
                events.CancelEvent(EVENT_SPELL_SEPSIS);
                events.CancelEvent(EVENT_SPELL_POISON_BOLT_VOLLEY);
                events.CancelEvent(EVENT_LINK_PLAYERS);

                DoCastSelf(SPELL_POISON_BEAM, false);

                ++PoisonBeamTick;

                if (PoisonBeamTick >= 50) {
                    
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->InterruptNonMeleeSpells(false);
                    events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, Milliseconds(50000));
                    me->SetTarget(PB_target->GetGUID());
                    //me->RemoveAurasDueToSpell(SPELL_FREEZE_ANIM);
                    isSpinning = false;
                    if(Phase == PHASE_TWO) {
                        events.ScheduleEvent(EVENT_SPELL_POISON_NOVA, Milliseconds(15000));
                        events.ScheduleEvent(EVENT_SPELL_NEUROTOXIN, Milliseconds(urand(5000, 10000)));
                        events.ScheduleEvent(EVENT_SPELL_SEPSIS, Milliseconds(11000));
                        events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, Milliseconds(30000));
                        events.ScheduleEvent(EVENT_SPELL_SPAWN_MUSHROOMS, Milliseconds(10000));
                    }
                    else if (Phase == PHASE_THREE) {
                        events.ScheduleEvent(EVENT_SPELL_POISON_NOVA, Milliseconds(5000));
                        events.ScheduleEvent(EVENT_SPELL_NEUROTOXIN, Milliseconds(urand(5000, 10000)));
                        events.ScheduleEvent(EVENT_SPELL_SEPSIS, Milliseconds(11000));
                        events.ScheduleEvent(EVENT_SPELL_POISON_BEAM_CAST, Milliseconds(20000));
                        events.ScheduleEvent(EVENT_SPELL_SPAWN_MUSHROOMS, Milliseconds(10000));
                        events.ScheduleEvent(EVENT_LINK_PLAYERS, Milliseconds(2000));
                    }
                }
                else {
                    events.Repeat(Milliseconds(300));
                }
                break;
            }
            case EVENT_SPELL_SPAWN_MUSHROOMS: {
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
                spawnedShroom->EngageWithTarget(Shroom_target);
                spawnedShroom->SetTarget();
                events.Repeat(Milliseconds(8000));
                auraStacks++;
                break;
            }
                                            
            case EVENT_LINK_PLAYERS: {
                Map* map = me->GetMap();
                if (map)
                {
                    Map::PlayerList const& PlayerList = map->GetPlayers();
                    auto i = PlayerList.begin();

                    while (i != PlayerList.end())
                    {
                        Player* player = i->GetSource();

                        // Pre-increment iterator for safe usage inside the loop
                        ++i;

                        if (!player)
                            continue;

                        // Skip dead players or out of range
                        if (!player->IsAlive() || me->GetDistance(player) > 100.0f)
                            continue;

                        // Skip tank (current victim)
                        if (Player* tank = me->GetThreatMgr().GetCurrentVictim()->ToPlayer())
                        {
                            if (player == tank)
                                continue;
                        }

                        playersInRange.push_back(player);
                    }
                }


                if (playersInRange.size() >= 3)
                {
                    std::list<Player*>::iterator it = playersInRange.begin();
                    std::advance(it, urand(0, playersInRange.size() - 1));
                    target_link1 = *it;
                    if(!target_link1){
                        LOG_DEBUG("module", "target_link1 is null");
                        return;
                    }
                    me->CastSpell(target_link1, SPELL_LINK_PLAYERS, false);

                    std::list<Player*>::iterator it2 = playersInRange.begin();
                    std::advance(it2, urand(0, playersInRange.size() - 1));
                    target_link2 = *it2;
                    if(!target_link2){
                        LOG_DEBUG("module", "target_link2 is null");
                        return;
                    }
                    if(target_link1 == target_link2) {
                        LOG_DEBUG("module", "target_link1 == target_link2, skipping link");
                        events.ScheduleEvent(EVENT_LINK_PLAYERS, Milliseconds(0));
                        return;
                    }
                    me->CastSpell(target_link2, SPELL_LINK_PLAYERS, false);
                    target_link1->CastSpell(target_link2, SPELL_LINK_PLAYERS_VISUAL, true);
                    target_link2->CastSpell(target_link1, SPELL_LINK_PLAYERS_VISUAL, true);
                    events.ScheduleEvent(EVENT_LINK_PLAYERS_DAMAGE, Milliseconds(0));
                    events.ScheduleEvent(EVENT_LINK_PLAYERS_DONE, 15s);
                }
                events.Repeat(Milliseconds(30000));
                break;
            }
            case EVENT_LINK_PLAYERS_DAMAGE: {
                if (target_link1->IsAlive() && target_link2->IsAlive()) {
                    float distance = target_link1->GetDistance(target_link2->GetPosition());
                    
                    if (distance > 10.0f) {
                        if (!target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) me->AddAura(SPELL_LINK_PLAYERS_DAMAGE, target_link1);
                        if (!target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) me->AddAura(SPELL_LINK_PLAYERS_DAMAGE, target_link2);
                        if (target_link1->HasAura(SPELL_AURA_DMG_LINK)) target_link1->RemoveAura(SPELL_AURA_DMG_LINK);
                        if (target_link2->HasAura(SPELL_AURA_DMG_LINK)) target_link2->RemoveAura(SPELL_AURA_DMG_LINK);
                    }
                    else {
                        if (target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link1->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                        if (target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link2->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                        target_link1->CastSpell(target_link1, SPELL_AURA_DMG_LINK, true);
                        target_link2->CastSpell(target_link2, SPELL_AURA_DMG_LINK, true);
                    }
                }
                events.Repeat(Milliseconds(100));
                break;
            }
            case EVENT_LINK_PLAYERS_DONE: {
                if (target_link1->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link1->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                if (target_link2->HasAura(SPELL_LINK_PLAYERS_DAMAGE)) target_link2->RemoveAura(SPELL_LINK_PLAYERS_DAMAGE);
                if (target_link1->HasAura(SPELL_LINK_PLAYERS)) target_link1->RemoveAura(SPELL_LINK_PLAYERS);
                if (target_link2->HasAura(SPELL_LINK_PLAYERS)) target_link2->RemoveAura(SPELL_LINK_PLAYERS);
                events.CancelEvent(EVENT_LINK_PLAYERS_DAMAGE);
                break;
            }
            
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            events.Reset();
            LOG_DEBUG("module", "Boss death");
            // Implement boss death behavior
            Talk(SAY_DEATH);
            LOG_DEBUG("module", "talked");
            RemoveMushrooms();
            LOG_DEBUG("module", "remvoed shroom");
            //events.ScheduleEvent(EVENT_DESPAWN, Milliseconds(10000));
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
        Unit* spawnedShroom;
        Player* target_link1, * target_link2;
        std::list<Player*> playersInRange;
    };



    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_eloxinAI(creature);
    }
};

class eloxin_mushroom : public CreatureScript
{
public:
    eloxin_mushroom() : CreatureScript("eloxin_mushroom") {}

    struct eloxin_mushroomAI : public ScriptedAI
    {
        eloxin_mushroomAI(Creature* creature) : ScriptedAI(creature) {
        }

        void JustEngagedWith(Unit* /*who*/) override {
            if (me->GetEntry() == MUSHROOM_GREEN) {
                me->SetHealth(1);
                events.ScheduleEvent(EVENT_SPELL_MUSHROOM_GREEN_PULSE, Milliseconds(500));
            }

            if (me->GetEntry() == MUSHROOM_BLUE) {
                events.ScheduleEvent(EVENT_SPELL_MUSHROOM_BLUE_PULSE, Milliseconds(1000));
            }

        }


        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_SPELL_MUSHROOM_BLUE_PULSE:
                DoCast(SPELL_MUSHROOM_BLUE_PULSE);
                events.Repeat(Milliseconds(1000));
                break;
            case EVENT_SPELL_MUSHROOM_GREEN_PULSE:
                if (me->IsFullHealth()) {
                    DoCast(SPELL_MUSHROOM_GREEN_DEATH);
                    me->DespawnOrUnsummon();
                }
                else events.Repeat(Milliseconds(500));
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
    EloxinSummoner() : GameObjectScript("EloxinSummoner") {}



    struct EloxinSummonerAI : public GameObjectAI
    {
        EloxinSummonerAI(GameObject* go) : GameObjectAI(go) {}

        virtual void SummonedCreatureDespawn(Creature* /*summon*/) override {
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
