#pragma once

//V241 for VS2008

//General use
namespace UpgradeId {
	enum Enum {
		TerranInfantryArmor = 0x00,		//
		TerranVehiclePlating = 0x01,	//
		TerranShipPlating = 0x02,		//
		ZergCarapace = 0x03,			//
		ZergFlyerCaparace = 0x04,		//
		ProtossArmor = 0x05,			//
		ProtossPlating = 0x06,			//
		TerranInfantryWeapons = 0x07,	//
		TerranVehicleWeapons = 0x08,	//
		TerranShipWeapons = 0x09,		//
		ZergMeleeAttacks = 0x0A,		//
		ZergMissileAttacks = 0x0B,		//
		ZergFlyerAttacks = 0x0C,		//
		ProtossGroundWeapons = 0x0D,	//
		ProtossAirWeapons = 0x0E,		//
		ProtossPlasmaShields = 0x0F,	//
		U238Shells = 0x10,				// Marine attack range
		IonThrusters = 0x11,			// Vulture movespeed
		FusionSealer = 0x12,			// SCV repair rate
		TitanReactor = 0x13,			// Science Vessel energy
		OcularImplants = 0x14,			// Ghost sight range
		MoebiusReactor = 0x15,			// Ghost energy
		ApolloReactor = 0x16,			// Wraith energy
		ColossusReactor = 0x17,			// Battlecruiser energy
		VentralSacs = 0x18,				// Overlord capacity
		Antennae = 0x19,				// Overlord sight range
		PneumatizedCarapace = 0x1A,		// Overlord movespeed
		MetabolicBoost = 0x1B,			// Zergling movespeed
		AdrenalGlands = 0x1C,			// Zergling attackspeed
		MuscularAugments = 0x1D,		// Hydralisk movespeed
		GroovedSpines = 0x1E,			// Hydralisk attack range
		GameteMeiosis = 0x1F,			// Queen energy
		MetasynapticNode = 0x20,		// Defiler energy
		SingularityCharge = 0x21,		// Strider attack range
		LegEnhancements = 0x22,			// Zealot movespeed
		ReinforcedScarabs = 0x23,		// Scarab armor and damage
		ReaverCapacity = 0x24,			// Reaver Scarab capacity
		GraviticDrive = 0x25,			// Shuttle movespeed
		SensorArray = 0x26,				// Observer sight range
		GraviticBoosters = 0x27,		// Observer movespeed
		KhaydarinAmulet = 0x28,			// High Templar energy
		ApialSensors = 0x29,			// Scout sight range
		GraviticThrusters = 0x2A,		// Scout movespeed
		AuxilliaryHangars = 0x2B,		// Carrier Interceptor capacity
		KhaydarinCore = 0x2C,			// Arbiter energy
		RapidReconstitution = 0x2D,		// Colony HP regeneration rate
		PrismaticShield = 0x2E,			// Photon Cannon shield armor
		ArgusJewel = 0x2F,				// Corsair energy
		Overcharge = 0x30,				// Shaman slow on attack
		ArgusTalisman = 0x31,			// Dark Archon energy
		ApollyoidAdhesive = 0x32,		// Firebat normal damage to buildings
		CaduceusReactor = 0x33,			// Medic energy
		ChitinousPlating = 0x34,		// Ultralisk armor
		AnabolicSynthesis = 0x35,		// Ultralisk movespeed
		CharonBooster = 0x36,			// Goliath attack range
		SignifyMalice = 0x37,			// Hierophant attack speed slow
		BladeVortex = 0x38,				// Legionnaire teleport to target
		Relocators = 0x39,				// Scarab "fly" behavior
		MiningEfficiency = 0x3A,		// Income multiplier
		KhaydarinDrive = 0x3B,			// Starcaller energy
		GlobalUpgrade60 = 0x3C,			// Unused 60 // DO NOT USE
		None = 0x3D,

		// extended upgrades \\

		RestorativeNanites = 62,		// Shaman mech restore on full repair
		PlasteelServos = 63,			// Cyprian movement speed
		OdysseusReactor = 64,			// Comsat Station energy
		Capacitors = 65,				// Wraith cloaked attack speed
		AblativeWurm = 66,				// Mutalisk bounce damage
		WardensMark = 67,				// Clarion weapon debuff for allied shieldsteal
		InnervatingMatrices = 68,		// Defensive Matrix heal
		AchernusReactor = 69,			// Azazel energy and energy vent passive
		UmojanBatteries = 70,			// Yamato Gun damage
		RammingSpeed = 71,				// Terran structure liftoff speed at the cost of HP
		AugustgradsRevenge = 72,		// Nuclear Missile damage and temp unbuildable terrain
		TendrilAmitosis = 73,			// Sunken Colony extra attack
		NydalTransfusion = 74,			// Nydus Canal place off creep
		VitalEncephalon = 75,			// Gorgoleth energy
		RaveningMandible = 76,			// Gorgoleth attack speed per energy
		Metastasis = 77,				// Plague transmit on death
		Heatshield = 78,				// Firebat HP
		SequesterPods = 80,				// Missile Turret range
		Reservations = 81,				// Terran Building extra addon spot
		RadiationShockwave = 82,		// Irradiate EMP on death
		ExpandedHull = 83,				// Shuttle transport capacity
		EnsnaringBrood = 84,			// Broodling/Infested Terran ensnare on attack/death
		KaumodakiReactor = 85,			// Savant energy and ability area increase
		EntropyGauntlets = 86,			// Savant Energy Decay area damage
		ChaosMatter = 87,				// Exemplar bounce during Khaydarin Eclipse
		FinalHour = 88,					// Augur projectile death
		ForeCastle = 89,				// Battlecruiser forward-facing armor
		TectonicClaws = 90,				// Lurker burrow speed
		SaberSiphon = 91,				// Ultralisk lifesteal
		KaiserRampage = 92,				// Ultralisk movement and attack speed on kill
		CorrosiveDispersion = 93,		// Basilisk attack range on creep
		SynapticGrowth = 94,			// Chasmal Colony energy
		LastOrders = 95,				// Scout crash on death
		TemporalBatteries = 96,			// Protoss Building shield regen (2x Pylon)
		KhaydarinGenerator = 97,		// Shield Battery and Bulwark energy
		CausticFission = 98,			// Basilisk bounce
		TaldarinsGrace = 99,			// Strider shield armor
		EphemeralBlades = 100,			// Hallucination damage
		Ascendancy = 101,				// High Templar energy and bonus energy on spell kill
		GroundZero = 102,				// Nuclear Missile radiation
		AdrenalinePacks = 103,			// Medic and Shaman support-armor on heal
		GraviticAttenuator = 104,		// Unused upgrade
		PeerReviewedDestruction = 105,	// Science Vessel paint nuke target
		PassengerInsurance = 106,		// Dropship acceleration
		DivineClarity = 107,			// Clarion +1 shield armor while above 50% shields
		AstralAegis = 108,				// Archon support-shield on death
		Skyforge = 109,					// Interceptor build time
		AdrenalSurge = 110,				// Vorvaling movement speed and homing
		KhaydarinAnchor = 111,			// Warp Relay energy
		PowerFlux = 112,				// Protoss Building upgrade speed (3x Pylon)
		KhaydarinEclipse = 113,			// Exemplar siege mode
		SomaticImplants = 114,			// Ghost movespeed while cloaked
		Hotdrop = 115,					// Dropship unload speed
		LitheOrganelle = 117,			// Mutalisk extra bounce
		CriticalMass = 118,				// Devourer Corrosive Spores explosion
		AcridDisplacement = 119,		// Guardian damage field per projectile travel time
		SeismicSpines = 120,			// Lurker projectile return to attacker
		Incubators = 121,				// Broodling Broodspawn on kill
		Autovitality = 122,				// Robotic unit shield capacity
		IonPropulsion = 123,			// Vulture "fly" behavior
		TempoChange = 124,				// Madrigal weapon switch // UNUSED
		RequiemAfterburners = 125,		// Madrigal missile speed
		HammerfallShells = 126,			// Siege Tank splash radius
		SunchaserMissiles = 127,		// Centaur secondary explosion
		KnockoutDrivers = 128,			// Southpaw speed slow
		BabylonPlating = 129,			// Siege Tank armor bonus in tank mode

		// relics \\

		Tempest = 200,					// 
		Penance = 201,					// 
		BlindJudge = 202,				// 
		Emperor = 203,					// 
		MindForge = 204,				// 
		Boltmaker = 205,				// 
		Artificer = 206,				// 
		Sentence = 207,					// 
		UmberMaw = 208,					// 
		Anthelion = 209,				// 
		EndlessVigil = 210,				// 
		ImmortalFortress = 211,			// 
		Intervention = 212,				// 
		PretadorsCloak = 213,			// 
		AstralAnchor = 214,				// 
		Thaumaturge = 215,				// 
		Armor7 = 216,					// 
		Armor8 = 217,					// 
		Armor9 = 218,					// 
		Armor10 = 219,					// 
		LightOfSaalok = 220,			// 
		CrestOfAdun = 221,				// 
		BlessedSummons = 222,			// 
		Heartseeker = 223,				// 
		ShadeSigil = 224,				// 
		BindingCrucible = 225,			// 
		VirtuousAttentuator = 226,		// 
		HolyEmber = 227,				// 
		Stormpeak = 228,				// 
		SirensClaw = 229,				// 
		SpiritEmblem = 230,				// 
		Augment11 = 231,				// 
		Augment12 = 232,				// 
		Augment13 = 233,				// 
		Augment14 = 234,				// 
		Augment15 = 235,				// 
		Augment16 = 236,				// 
		Augment17 = 237,				// 
		Augment18 = 238,				// 
		Augment19 = 239,				// 
		Augment20 = 240,				// 

	#ifdef HYDRA
		SilentStrikes = 79,				// Dark Templar attack speed on kill
		Malediction = 116,				// Maelstrom Feedback and slow
	#endif
		
	#ifdef DELIVERANCE
		AstralBlessing = 150,			// Ecclesiast support-shield behavior
		AstralTempest = 151,			// Ecclesiast support-attack speed behavior
		DeathlessProcession = 152,		// Amaranth shieldsteal
	#endif
	};
}

//For use with upgrades_dat::MaximumUpgSc[] and upgrades_dat::CurrentUpgSc[]
namespace ScUpgrade {
	enum Enum {
		TerranInfantryArmor = 0,
		TerranVehiclePlating = 1,
		TerranShipPlating = 2,
		ZergCarapace = 3,
		ZergFlyerCaparace = 4,
		ProtossArmor = 5,
		ProtossPlating = 6,
		TerranInfantryWeapons = 7,
		TerranVehicleWeapons = 8,
		TerranShipWeapons = 9,
		ZergMeleeAttacks = 10,
		ZergMissileAttacks = 11,
		ZergFlyerAttacks = 12,
		ProtossGroundWeapons = 13,
		ProtossAirWeapons = 14,
		ProtossPlasmaShields = 15,
		U_238Shells = 16,
		IonThrusters = 17,
		BurstLasers = 18, // (Unused)
		TitanReactor = 19, // (SV +50)
		OcularImplants = 20,
		MoebiusReactor = 21, // (Ghost +50)
		ApolloReactor = 22, // (Wraith +50)
		ColossusReactor = 23, // (BC +50)
		VentralSacs = 24,
		Antennae = 25,
		PneumatizedCarapace = 26,
		MetabolicBoost = 27,
		AdrenalGlands = 28,
		MuscularAugments = 29,
		GroovedSpines = 30,
		GameteMeiosis = 31, // (Queen +50)
		MetasynapticNode = 32, // (Defiler +50)
		SingularityCharge = 33,
		LegEnhancements = 34,
		ScarabDamage = 35,
		ReaverCapacity = 36,
		GraviticDrive = 37,
		SensorArray = 38,
		GraviticBoosters = 39,
		KhaydarinAmulet = 40, // (HT +50)
		ApialSensors = 41,
		GraviticThrusters = 42,
		AuxilliaryHangars = 43,
		KhaydarinCore = 44, // (Arbiter +50)
		UnusedUpgrade45 = 45,
	};
}

//For use with upgrades_dat::MaximumUpgBw[] and upgrades_dat::CurrentUpgBw[]
namespace BwUpgrade {
	enum Enum {
		UnusedUpgrade46 = 0,
		ArgusJewel = 1, // (Corsair +50)
		UnusedUpgrade48 = 2,
		ArgusTalisman = 3, // (Dark Archon +50)
		UnusedUpgrade50 = 4,
		CaduceusReactor = 5, // (Medic +50)
		ChitinousPlating = 6,
		AnabolicSynthesis = 7,
		CharonBooster = 8,
		SignifyMalice = 9,
		BattleHarmony = 10,
		Relocators = 11,
		AIMiningEfficiency = 12,
		KhaydarinDrive = 13, // (Starcaller +50)
		GlobalUpgrade60 = 14, // (unused 60)
	};
}
