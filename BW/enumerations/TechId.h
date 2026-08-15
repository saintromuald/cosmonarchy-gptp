#pragma once

namespace TechId {
    enum Enum {
        // Cosmonarchy \\

    // Shared
        TargetGround						= 17, // Ensnare | used for: Wyvern Land Grab
        CancelNewAbility					= 26, // Unknown Tech 26 | used for: Wyvern Land Grab, Didact Dimensional Recall, Controlled Demolition

    // Terran
        GoskaLobotomyGrenade				=  1, // LobotomyGrenade
        ControlledDemolition				=  2, // EMP Shockwave
        DivineDiscipline                    = 10, // Ghost Personnel Cloaking
        RejoinderOfService                  = 12, // Infestation
        ApostleLazarusAgent					= 32, // Lakizilisk Aspect
        EmergencyDetonation					= 39, // Unknown Tech 39
        AzazelSublimeShepherd				= 40, // Unknown Tech 40
        HypnagogueThroughTheLookingGlass	= 42, // Unknown Tech 42
        TritonLobotomyWarhead   			= 46, // Unknown Tech 46
        FacadePryingEyes                    = 47,

    // Protoss
        StarSovereignTheGriefOfAllGods		= 29, // Feedback

    // Zerg
        MatralethEnsnaringBrood				= 13, // Spawn Broodlings
        ZergBlissfulEmbrace					= 35, // Unknown Tech 35
        MalkothrokorHiveTissue              = 45,
        StroluumSkryspawn					= 51,

        LarvalSubmenuSelectControl			= 28, // Dark Archon Meld
        LarvalSubmenuSelectSwarm			= 33, // Unknown Tech 33
        LarvalSubmenuSelectAssault			= 38, // Unknown Tech 38

    // Askosi
        AskosiRapidRendezvous               = 9,  // Wraith Cloaking Field
        AskosiRemoteHijack                  = 16, // Consume
        AskosiPreylock					    = 24, // Restoration
        AskosiRematerialize     			= 23, // Archon Warp
        AskosiHijack                        = 36, // Unknown Tech 36
        AskosiAshbind                       = 37, // Unknown Tech 37
        AskosiSalvage						= 41, // Unknown Tech 41
        AskosiPuppeteer                     = 52,

        // Brood War \\

        Stimpacks				=  0,
    //  Lockdown				=  1,
    //  EMPShockwave			=  2,
        TerranMines				=  3,
        ScannerSweep			=  4, // (default)
        TankSiegeMode			=  5,
        DefensiveMatrix			=  6, // (default)
        Irradiate				=  7,
        YamatoGun				=  8,
        CloakingField			=  9,
        PersonnelCloaking		= 10,
        Burrowing				= 11,
        Infestation				= 12, // (default)
        SpawnBroodlings			= 13,
        CarapaceSwarm			= 14, // (default)
        Plague					= 15,
        Consume					= 16,
        Ensnare					= 17,
        MatralethParasite		= 18, // (default)
        PsionicStorm			= 19,
        Hallucination			= 20,
        DimensionalRecall		= 21,
        StasisField				= 22,
        ArchonWarp				= 23, // (default)
        Restoration				= 24,
        LanifectDisruptionField	= 25,
        UnusedTech26			= 26, // (unused)
        MindTyrantMaledictus	= 27,
        DarkArchonMeld			= 28, // (default)
        Feedback				= 29, // (default)
        LuminaryAuralAssault	= 30,
        MindTyrantMaelstrom		= 31,
        LurkerAspect			= 32,
        UnusedTech33			= 33, // (default; possibly liftoff)
        Healing					= 34, // (default)
        UnusedTech35			= 35, // (unused)
        UnusedTech36			= 36, // (unused)
        UnusedTech37			= 37, // (unused)
        UnusedTech38			= 38, // (unused)
        UnusedTech39			= 39, // (unused)
        UnusedTech40			= 40, // (unused)
        UnusedTech41			= 41, // (unused)
        UnusedTech42			= 42, // (unused)
        UnusedTech43			= 43, // (unused)
        None					= 44,
        NuclearStrike			= 45,
        UnknownLastEntry		= 46, // based on bwapi/BWMemoryEdit/Enums/TechType.cs
    };
}

// For use with techdata_dat::AvailableSC[] and techdata_dat::ResearchedBw[]
namespace ScTech {
    enum Enum {
        Stimpacks			=  0,
        Lockdown			=  1,
        EMPShockwave		=  2,
        SpiderMines			=  3,
        ScannerSweep		=  4,	// (default)
        TankSiegeMode		=  5,
        DefensiveMatrix		=  6,	// (default)
        Irradiate			=  7,
        YamatoGun			=  8,
        CloakingField		=  9,
        PersonnelCloaking	= 10,
        Burrowing			= 11,
        Infestation			= 12,	// (default)
        SpawnBroodlings		= 13,
        CarapaceSwarm		= 14,	// (default)
        Plague				= 15,
        Consume				= 16,
        Ensnare				= 17,
        MatralethParasite	= 18,	// (default)
        PsionicStorm		= 19,
        Hallucination		= 20,
        DimensionalRecall	= 21,
        StasisField			= 22,
        ArchonWarp			= 23,	// (default)
    };
}

//For use with techdata_dat::AvailableBW[] and techdata_dat::ResearchedBw[]
namespace BwTech {
    enum Enum {
        Restoration					=  0,
        LanifectDisruptionField     =  1,
        UnusedTech26				=  2,	// (unused)
        MindTyrantMaledictus						=  3,
        DarkArchonMeld				=  4,	// (default)
        Feedback					=  5,	// (default)
        LuminaryAuralAssault		=  6,
        MindTyrantMaelstrom         =  7,
        LurkerAspect				=  8,
        UnusedTech33				=  9,	// (default; possibly liftoff)
        Healing						= 10,	// (default)
        UnusedTech35				= 11,	// (unused)
        UnusedTech36				= 12,	// (unused)
        UnusedTech37				= 13,	// (unused)
        UnusedTech38				= 14,	// (unused)
        UnusedTech39				= 15,	// (unused)
        UnusedTech40				= 16,	// (unused)
        UnusedTech41				= 17,	// (unused)
        UnusedTech42				= 18,	// (unused)
        UnusedTech43				= 19,	// (unused)
    };
}