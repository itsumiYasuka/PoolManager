#define NOMINMAX
#include "rage.hpp"
#include "joaat.h"
#include <Hooking.h>
#include <MinHook.h>
#include <iostream>
#include <fstream>
#include "Dependencies/ankerl/unordered_dense.h"

#pragma comment(lib,"Dependencies/minhook/libMinHook.x64.lib")

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char* (&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ joaat::generate(list[i]), list[i] });
		}
	}

	inline std::string LookupHashString(uint32_t hash)
	{

		if (hash == NULL)
		{
			return "unknown";
		}

		auto it = m_lookupList.find(hash);
		if (it != m_lookupList.end())
		{
			return std::string(it->second);
		}

		char buffer[32];
		snprintf(buffer, std::size(buffer), "0x%08X", hash);
		return buffer;
	}

	inline std::string LookupHash(uint32_t hash)
	{
		if (hash == NULL)
		{
			return "unknown";
		}

		char buffer[32];
		snprintf(buffer, std::size(buffer), "0x%08X", hash);
		return buffer;
	}

private:
	ankerl::unordered_dense::map<uint32_t, std::string_view> m_lookupList;
};

int LogPercentUsageWarning = GetPrivateProfileInt("POOL_SETTINGS", "LogPercentUsageWarning", 0, ".\\PoolManager.toml");
int LogPercentUsageWarningAmount = GetPrivateProfileInt("POOL_SETTINGS", "LogPercentUsageWarningAmount", 50, ".\\PoolManager.toml");
int LogInitialPoolAmounts = GetPrivateProfileInt("POOL_SETTINGS", "LogInitialPoolAmounts ", 0, ".\\PoolManager.toml");

static std::string LastPoolLogged;
static int LastSizeLogged;

bool clearedLogs = false;

static void cleanUpLogs()
{
	if (!clearedLogs)
	{
		if (LogInitialPoolAmounts != 0)
		{
			std::ofstream outfile;
			outfile.open("PoolManager_Startup.log", std::ofstream::out | std::ofstream::trunc);
			outfile.close();
		}

		if (LogPercentUsageWarning != 0)
		{
			std::ofstream outfile;
			outfile.open("PoolManager_UsageWarning.log", std::ofstream::out | std::ofstream::trunc);
			outfile.close();
		}

		clearedLogs = true;
	}
}

static std::map<uint32_t, rage::fwBasePool*> g_pools;
static std::map<rage::fwBasePool*, uint32_t> g_inversePools;
static std::map<std::string, uint32_t> g_intPools;
static std::multimap<uint32_t, std::string> g_intPoolsMulti;

static const char* poolEntriesTable[] = {
"animatedbuilding",
"attachmentextension",
"audioheap",
"audscene",
"auddynmixpatch",
"auddynmixpatchsettings"
"avoidancevolumes",
"cbirdcurvecontainer",
"blendshapestore",
"building",
"ccarryconfigtargetclipsetrequester",
"ctimedcarryconfigtargetclipsetrequester",
"cactionconfiginfo",
"caimsolver",
"caimsolver::internalstate",
"cambientmaskvolume",
"cambientmaskvolumeentity",
"cambientmaskvolumedoor",
"carmsolver",
"clegposturesolver",
"carmposturesolver",
"cimpulsereactionsolver",
"cmountedlegsolver",
"cmountedlegsolverproxy",
"carmiksolver",
"cbalancesolver",
"cclimbsolver",
"cbodylookiksolver",
"cbodylookiksolver::internalstate",
"cbodylookiksolverproxy",
"cbodyrecoiliksolver",
"clegiksolver",
"clegiksolverstate",
"clegiksolverproxy",
"cquadlegiksolver",
"cquadlegiksolverproxy",
"crootslopefixupiksolver",
"cstirrupsolver",
"ccontoursolver",
"ccontoursolverproxy",
"ctorsoreactiksolver",
"ctorsovehicleiksolver",
"ctorsovehicleiksolverproxy",
"cupperbodyblend",
"cupperbodyblend::cbodyblendbonecache",
"cbodyaimsolver",
"cbodyaimsolver::internalstate",
"ctwobonesolver",
"cposefixupsolver",
"cvehicleturretsolver",
"cquadrupedinclinesolver",
"cquadrupedinclinesolverproxy",
"cfullbodysolver",
"cgamescripthandler",
"chighheelsolver",
"cbodydampingsolver",
"cbodyreachsolver",
"canimalattackgroup",
"canimalflock",
"canimaltargeting",
"canimaltuning",
"canimalunalertedgroup",
"canimalgroup",
"canimalgroupmember",
"cflocktuning",
"cpopzonespawner",
"cmodelsetspawner",
"animscenestore",
"carrec",
"cvehiclecombatavoidancearea",
"ccargen",
"ccombatdirector",
"cpedsharedtargeting",
"ccombatinfo",
"ccombatsituation",
"cbarbrawler",
"ccoverfinder",
"caimhelper",
"cinventoryitem",
"cweaponcomponentitem",
"cweaponitem",
"cammoitem",
"csatchelitem",
"cclothingitem",
"chorseinventoryitem",
"ccharacteritem",
"chorseequipmentinventoryitem",
"ccoachinventoryitem",
"ccampitem",
"cdogitem",
"ccrimeobserver",
"cscenariorequest",
"cscenariorequesthandler",
"cscenariorequestresults",
"cscenarioinfo",
"ctacticalanalysis",
"ctaskusescenarioentityextension",
"cterrainadaptationhelper",
"canimscenehelper",
"animstore",
"clipstore",
"cgamescriptresource",
"clothstore",
"ccombatmeleegroup",
"combatmountedmanager_attacks",
"compentity",
"cpedinventory",
"persistentlootabledata",
"lootactionfinderfsm",
"managedlootableentitydata",
"carryactionfinderfsm",
"motionstore",
"compositelootableentitydefinst",
"carriableextension",
"centitygameinfocomponent",
"cemotionallocohelper",
"cgameownership",
"fwanimationcomponent",
"canimationcomponent",
"cdynamicentityanimationcomponent",
"cobjectanimationcomponent",
"cpedanimationcomponent",
"cvehicleanimationcomponent",
"canimatedbuildinganimationcomponent",
"curvelib::curve",
"fwcreaturecomponent",
"fwanimdirectorcomponentcreature",
"fwanimdirectorcomponentmotiontree",
"fwanimdirectorcomponentmove",
"fwanimdirectorcomponentfacialrig",
"fwanimdirectorcomponentcharactercreator",
"fwanimdirectorcomponentextraoutputs",
"fwanimdirectorcomponentpose",
"fwanimdirectorcomponentexpressions",
"fwanimdirectorcomponentragdoll",
"fwanimdirectorcomponentparent",
"fwanimdirectorcomponentparent_parent",
"fwanimdirectorcomponentreplay",
"canimdirectorcomponentik",
"fwanimdirector",
"crcreaturecomponentskeleton",
"crcreaturecomponentextradofs",
"crcreaturecomponentexternaldofs",
"crcreaturecomponentcloth",
"crcreaturecomponentshadervars",
"crcreaturecomponentphysical",
"crcreaturecomponenthistory",
"crcreaturecomponentparticleeffect",
"crexpressionplayer",
"crrelocatableheapsize",
"crrelocatableasynccompactsize",
"crrelocatablemapslots",
"crframeacceleratorheapsize",
"crframeacceleratormapslots",
"crframeacceleratorentryheaders",
"crweightsetacceleratorheapsize",
"crweightsetacceleratormapslots",
"crweightsetacceleratorentryheaders",
"mvpagebuffersize",
"crmtnodefactorypool",
"crikheap",
"fwmtupdatescheduleroperation",
"cpedcreaturecomponent",
"cpedanimalcomponent",
"cpedanimalearscomponent",
"cpedanimaltailcomponent",
"cpedanimalaudiocomponent",
"cpedavoidancecomponent",
"cpedbreathecomponent",
"cpedclothcomponent",
"cpedcorecomponent",
"cpeddamagemodifiercomponent",
"cpedmeleemodifiercomponent",
"cpeddrivingcomponent",
"cpedfacialcomponent",
"cpedfootstepcomponent",
"cpedgameplaycomponent",
"cpedattributecomponent",
"cpeddistractioncomponent",
"cpeddummycomponent",
"cpedgraphicscomponent",
"cpedvfxcomponent",
"cpedhealthcomponent",
"cpedhorsecomponent",
"cpedhumanaudiocomponent",
"cpedeventcomponent",
"cpedintelligencecomponent",
"cpedinventorycomponent",
"cpedlookatcomponent",
"cpedmotioncomponent",
"cpedmotivationcomponent",
"cpedphysicscomponent",
"cpedprojdecalcomponent",
"cpedragdollcomponent",
"cpedscriptdatacomponent",
"cpedstaminacomponent",
"cpedtargetingcomponent",
"cpedthreatresponsecomponent",
"cpedtransportcomponent",
"cpedtransportusercomponent",
"cpedweaponcomponent",
"cpedweaponmanagercomponent",
"cpedvisibilitycomponent",
"cobjectautostartanimcomponent",
"cobjectbreakableglasscomponent",
"cobjectbuoyancymodecomponent",
"cobjectcollisiondetectedcomponent",
"cobjectcollisioneffectscomponent",
"cobjectdoorcomponent",
"cobjectdraftvehiclewheelcomponent",
"cobjectintelligencecomponent",
"cobjectnetworkcomponent",
"cobjectphysicscomponent",
"cobjectriverprobesubmissioncomponent",
"maxriverphysicsinsts",
"cobjectvehicleparentdeletedcomponent",
"cobjectweaponscomponent",
"cpairedanimationreservationcomponent",
"cvehiclecorecomponent",
"cvehicledrivingcomponent",
"cvehicleintelligencecomponent",
"cvehiclephysicscomponent",
"cvehicleweaponscomponent",
"cpickupdata",
"cprioritizedsetrequest",
"cprioritizeddictionaryrequest",
"cquadrupedreactsolver",
"croadblock",
"cstuntjump",
"csimulatedroutemanager::route",
"csquad",
"cutscenestore",
"cscriptentityextension",
"cscriptentityidextension",
"cvehiclechasedirector",
"cvehiclecliprequesthelper",
"cvolumelocationextension",
"cvolumeownerextension",
"cmeleecliprequesthelper",
"cgrapplecliprequesthelper",
"cactioncache",
"cgrabhelper",
"cfleedecision",
"cpointgunhelper",
"cthreatenedhelper",
"cstunthelper",
"cgpsnumnodesstored",
"cclimbhandholddetected",
"cambientflockspawncontainer",
"customshadereffectbatchtype",
"customshadereffectbatchslodtype",
"customshadereffectcommontype",
"customshadereffectgrasstype",
"customshadereffecttreetype",
"cwildlifespawnrequest",
"decorator",
"decoratorextension",
"drawablestore",
"dummy object",
"cpropsetobjectextension",
"cmapentityrequest",
"cmapdatareference",
"cpinmapdataextension",
"dwdstore",
"entitybatch",
"grassbatch",
"entitybatchbitset",
"tcbox",
"tcvolume",
"exprdictstore",
"flocksperpopulationzone",
"framefilterstore",
"fragmentstore",
"fwscriptguid",
"fwuianimationopbase",
"fwuianimationopinstancedatabase",
"fwuianimationtargetbase",
"fwuianimationvaluebase",
"fwuiblip",
"layoutnode",
"fwuivisualpromptdata",
"fwuiiconhandle",
"fwcontainerlod",
"gameplayerbroadcastdatahandler_remote",
"ccontainedobjectid",
"ccontainedobjectidsinfo",
"interiorinst",
"instancebuffer",
"interiorproxy",
"iplstore",
"itemset",
"itemsetbuffer",
"jointlimitdictstore",
"lastinstmatrices",
"bodydatadictstore",
"behaviordatadictstore",
"cpersistentcharacterinfo",
"cpersistentcharactergroupinfo",
"cpersistentcharacter",
"cperschargroup",
"cperscharhorse",
"cperscharvehicle",
"canimalattractor",
"canimaldispatch",
"cnavobstructionpath",
"volcylinder",
"volbox",
"volsphere",
"volaggregate",
"volnetdatastateprimitive",
"volnetdatastateaggregate",
"maxloadedinfo",
"maxloadrequestedinfo",
"activeloadedinfo",
"activepersistentloadedinfo",
"maxmanagedrequests",
"maxunguardedrequests",
"maxreleaserefs",
"known refs",
"clightentity",
"mapdataloadednode",
"mapdatastore",
"maptypesstore",
"metadatastore",
"navmeshes",
"netscriptserialisationplan_small",
"netscriptserialisationplan_large",
"netscriptserialisationplan_extralarge",
"networkdefstore",
"networkcrewdatamgr",
"networkscriptstatusmanager",
"object",
"naspeechinst",
"navocalization",
"fwactivemanagedwaveslotinterface",
"nafoliageentity",
"nafoliagecontactevent",
"objectdependencies",
"occlusioninteriorinfo",
"occlusionpathnode",
"occlusionportalentity",
"occlusionportalinfo",
"peds",
"cpedequippedweapon",
"pedroute",
"cweapon",
"cweaponcomponent",
"cweaponcomponentinfo",
"phinstgta",
"fragcacheentriesprops",
"fragcacheheadroom",
"maxclothcount",
"maxcachedropecount",
"maxropecount",
"maxvisibleclothcount",
"maxpresimdependency",
"maxsinglethreadedphysicscallbacks",
"maxsinglethreadedphysicscallbacks",
"worldupdateentities",
"maxfoliagecollisions",
"fraginstgta",
"physicsbounds",
"maxbroadphasepairs",
"cpickup",
"cpickupplacement",
"cpickupplacementcustomscriptdata",
"cregenerationinfo",
"portalinst",
"posematcherstore",
"pmstore",
"ptfxsortedentity",
"ptfxassetstore",
"quadtreenodes",
"scaleformstore",
"scaleformmgrarray",
"scriptbrains",
"scriptstore",
"srequest",
"staticbounds",
"cremotetaskdata",
"textstore",
"txdstore",
"vehicles",
"vehiclestreamrequest",
"vehiclestreamrender",
"vehiclestruct",
"handlingdata",
"wptrec",
"fwlodnode",
"ctask",
"ctasknetworkcomponent",
"cevent",
"cmoveobject",
"cmoveanimatedbuilding",
"atdscriptobjectnode",
"cremotescriptargs",
"fwdynamicarchetypecomponent",
"fwdynamicentitycomponent",
"fwentitycontainer",
"fwmatrixtransform",
"fwquaterniontransform",
"fwsimpletransform",
"scenariopointsandedgesperregion",
"scenariopointentity",
"scenariopointworld",
"maxnonregionscenariopointspatialobjects",
"maxtrainscenariopoints",
"maxscenarioprompts",
"maxscenariointeriornames",
"objectintelligence",
"vehiclescenarioattractors",
"aircraftflames",
"crelationshipgroup",
"cscenariopoint",
"cscenariopointchainuseinfo",
"cscenarioclusterspawnedtrackingdata",
"cspclusterfsmwrapper",
"cgroupscenario",
"fwarchetypepooledmap",
"ctaskconversationhelper",
"syncedscenes",
"animscenes",
"cpropmanagementhelper",
"cpropinstancehelper",
"cscenariopropmanager::pendingpropinfo",
"cscenariopropmanager::loadedpropinfo",
"cscenariopropmanager::uprootedpropinfo",
"cscenariopropmanager::activeschedule",
"cgamescripthandlernetwork",
"navmeshroute",
"ccustommodelboundsmappings::cmapping",
"ladderentities",
"stairsentities",
"cqueriabletaskinfo",
"cguidcomponent",
"chealthcomponent",
"cavoidancecomponent",
"cprojdecalcomponent",
"clightcomponent",
"clightshaftcomponent",
"ctransportcomponent",
"ckinematiccomponent",
"csubscriberentitycomponent",
"cportablecomponent",
"cunlock",
"clightgroupextensioncomponent",
"clightshaftextensioncomponent",
"cladderinfo",
"cladderinfoextensioncomponent",
"cfakedoorinfo",
"cfakedoorextension::fakedoorinfo",
"cfakedoorgroupextensioncomponent",
"cfakedoorextension",
"caudiocollisionextensioncomponent",
"caudioemitter",
"caudioeffectextensioncomponent",
"cfragobjectanimextensioncomponent",
"cprocobjattr",
"cprocobjectextensioncomponent",
"cswayableattr",
"cswayableextensioncomponent",
"cbuoyancyextensioncomponent",
"cexpressionextensioncomponent",
"cwinddisturbanceextensioncomponent",
"cstairsextension",
"cstairsextensioncomponent",
"cdecalattr",
"cdecalextensioncomponent",
"cobjectautostartanimextensioncomponent",
"cexplosionattr",
"cexplosionextensioncomponent",
"cobjectlinksextensioncomponent",
"cparticleattr",
"cparticleextensioncomponent",
"camvolumeextensioncomponent",
"cfogvolumeextensioncomponent",
"clothmanagerheapsize",
"scrglobals",
"orders",
"incidents",
"caicurvepoint",
"fraginstgta",
"ctasksequencelist",
"cvehiclestreamrequestgfx",
"chandlingobject",
"cpatrollink",
"cpatrolnode",
"pointroute",
"cspawnpointoverrideextension",
"ccollectioninfo",
"wheels",
"vehicleglasscomponententity",
"naenvironmentgroup",
"cbullet",
"cbullet::sbulletinstance",
"itemsetbuffer",
"itemset",
"tasksequenceinfo",
"ceventnetwork",
"scriptshapetestresult",
"cnamedpatrolroute",
"explosiontype",
"camsplinenode",
"clandinggear_qkdfqq",
"collision_5plvhjd",
"shapetesttaskdata",
"scenariopoint",
"ceventdecisionmaker",
"musicevent",
"musicaction",
"ccargenforscenarios",
"vehicleaudioentity",
#include "RDR3VtableList.h"
};

static RageHashList poolEntries(poolEntriesTable);

static rage::fwBasePool* SetPoolFn(rage::fwBasePool* pool, uint32_t hash)
{
	g_pools[hash] = pool;
	g_inversePools.insert({ pool, hash });

	return pool;
}

static void(*g_origPoolDtor)(rage::fwBasePool*);

static void PoolDtorWrap(rage::fwBasePool* pool)
{
	auto hashIt = g_inversePools.find(pool);

	if (hashIt != g_inversePools.end())
	{
		auto hash = hashIt->second;

		g_pools.erase(hash);
		g_inversePools.erase(pool);
	}

	return g_origPoolDtor(pool);
}

typedef std::int32_t* (*PoolAllocateWrap_t)(rage::fwBasePool*, uint64_t unk);
PoolAllocateWrap_t g_origPoolAllocate = nullptr;

std::int32_t* PoolAllocateWrap(rage::fwBasePool* pool, uint64_t unk)
{
	auto value = g_origPoolAllocate(pool, unk);


	if (LogPercentUsageWarning != 0)
	{
		if ((float)pool->GetCount() / (float)pool->GetSize() * 100.00f > LogPercentUsageWarningAmount)
		{
			auto it = g_inversePools.find(pool);

			uint32_t poolHash = 0;
			if (g_inversePools.count(pool) != 0)
				poolHash = it->second;

			std::string poolName = poolEntries.LookupHashString(poolHash);
			std::string poolNameHash = poolEntries.LookupHash(poolHash);

			auto comparisons = g_intPoolsMulti.equal_range(pool->GetSize());

			if (poolName == "unknown" && comparisons.first != comparisons.second)
			{
				poolName = "unknown - possible pool names with same value: ";;

				for (auto comparisonsr = comparisons.first; comparisonsr != comparisons.second; ++comparisonsr)
				{
					poolName = poolName + " " + comparisonsr->second;
				}
			}

			auto poolSize = pool->GetSize();
			auto poolCount = pool->GetCount();
			float percent = (float)poolCount / (float)poolSize;

			std::ofstream outfile;
			outfile.open("PoolManager_UsageWarning.log", std::ios_base::app);

			if (LastPoolLogged == poolName && LastSizeLogged == poolSize)
			{
				outfile << "(count:" << poolCount << "/"
					<< "size:" << poolSize << ") poolUsage :" << percent * 100.00f << "%" << std::endl;
			}
			else
			{
				outfile << std::endl << "poolName: " << poolName.c_str() << std::endl
					<< "poolHash: " << poolNameHash.c_str() << std::endl
					<< "poolSize: " << poolSize << std::endl
					<< "poolCount: " << poolCount << std::endl
					<< "poolUsage: " << percent * 100.00f << "%" << std::endl
					<< "poolPointer: " << pool << std::endl;
				LastPoolLogged = poolName;
				LastSizeLogged = poolSize;
			}
		}
	}

	if (!value)
	{
		auto it = g_inversePools.find(pool);

		uint32_t poolHash = 0;
		if (g_inversePools.count(pool) != 0)
			poolHash = it->second;

		std::string poolName = poolEntries.LookupHashString(poolHash);
		std::string poolNameHash = poolEntries.LookupHash(poolHash);

		auto comparisons = g_intPoolsMulti.equal_range(pool->GetSize());
		if (poolName == "unknown" && comparisons.first != comparisons.second)
		{
			poolName = "unknown - possible pool names with same value: ";;

			for (auto comparisonsr = comparisons.first; comparisonsr != comparisons.second; ++comparisonsr)
			{
				poolName = poolName + " " + comparisonsr->second;
			}
		}

		std::ofstream outfile;
		outfile.open("PoolManager_Crash.log", std::ios_base::app);
		outfile << "poolName: " << poolName.c_str() << std::endl
			<< "poolHash: " << poolNameHash.c_str() << std::endl
			<< "poolSize: " << pool->GetSize() << std::endl
			<< "poolPointer: " << pool << std::endl
			<< std::endl;
		outfile.close();

		char buff[256];
		std::string extraWarning;
		if (poolName.find("0x") == std::string::npos)
		{
			sprintf_s(buff, "\nYou need to raise %s's pool size in update_1.rpf/common/data/gameconfig.xml", poolName.c_str());
			extraWarning = buff;
		}

		sprintf_s(buff, "%s pool crashed the game! \nPool hash: %s \nCurrent pool size: %llu \nCrash saved to PoolManager_Crash.log %s", poolName.c_str(), poolNameHash.c_str(), pool->GetSize(), extraWarning.c_str());
		std::cout << buff;
		HWND hWnd = FindWindow("grcWindow", NULL);
		int msgboxID = MessageBox(hWnd, buff, "PoolManager.asi", MB_OK | MB_ICONERROR);

		switch (msgboxID)
		{
		case IDOK:
			exit(0);
			break;
		}
	}

	return value;
}

typedef std::uint32_t(*GetSizeOfPool_t)(VOID* _this, std::uint32_t poolHash, std::uint32_t defaultSize, __int64 _ScriptHookRDR2Stuff);
GetSizeOfPool_t g_origSizeOfPool = nullptr;

std::uint32_t GetSizeOfPool(VOID* _this, std::uint32_t poolHash, std::uint32_t defaultSize, __int64 _ScriptHookRDR2Stuff)
{
	auto value = g_origSizeOfPool(_this, poolHash, defaultSize, _ScriptHookRDR2Stuff);
	std::string poolName = poolEntries.LookupHashString(poolHash);
	std::string poolNameHash = poolEntries.LookupHash(poolHash);

	auto it = g_intPools.find(poolName);
	if (it == g_intPools.end())
	{
		g_intPools.insert({ poolName, value });
		if (LogInitialPoolAmounts != 0)
		{
			if (poolName == poolNameHash)
			{
				poolName = "unknown";
			}
			std::ofstream outfile;
			outfile.open("PoolManager_Startup.log", std::ios_base::app);
			outfile << "poolName: " << poolName.c_str() << std::endl
				<< "poolHash: " << poolNameHash.c_str() << std::endl
				<< "poolSize: " << value << std::endl
				<< std::endl;
		}
	}

	return value;
}

static struct MhInit
{
	MhInit()
	{
		MH_Initialize();
	}
} mhInit;

void InitializeMod()
{
	cleanUpLogs();

	auto registerPool = [](hook::pattern_match match, int callOffset, uint32_t hash, bool isAssetStore)
	{
		struct : jitasm::Frontend
		{
			uint32_t hash;
			uint64_t origFn;
			bool isAssetStore;

			void InternalMain() override
			{
				sub(rsp, 0x38);

				mov(rax, qword_ptr[rsp + 0x38 + 0x28]);
				mov(qword_ptr[rsp + 0x20], rax);

				mov(rax, qword_ptr[rsp + 0x38 + 0x30]);
				mov(qword_ptr[rsp + 0x28], rax);

				mov(rax, origFn);
				call(rax);

				mov(rcx, rax);

				if(isAssetStore == true)
					add(rcx, 0x38);

				mov(edx, hash);

				mov(rax, (uint64_t)&SetPoolFn);
				call(rax);

				add(rsp, 0x38);

				ret();
			}
		} *stub = new std::remove_pointer_t<decltype(stub)>();

		stub->hash = hash;
		stub->isAssetStore = isAssetStore;

		auto call = match.get<void>(callOffset);
		hook::set_call(&stub->origFn, call);
		hook::call(call, stub->GetCode());
	};

	auto registerPools = [&](hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			registerPool(match, callOffset, *match.get<uint32_t>(hashOffset), false);
		}
	};

	auto registerNamedPools = [&](hook::pattern& patternMatch, int callOffset, int nameOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			char* name = hook::get_address<char*>(match.get<void*>(nameOffset));
			registerPool(match, callOffset, joaat::generate(name), false);
		}
	};

	auto registerAssetPools = [&](hook::pattern& patternMatch, int callOffset, int nameOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			char* name = hook::get_address<char*>(match.get<void*>(nameOffset));
			registerPool(match, callOffset, joaat::generate(name), true);
		}
	};

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 8B D8 E8"), 51, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CD"), 41, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CE"), 45, 1);

	registerNamedPools(hook::pattern("48 8D 15 ? ? ? ? 33 C9 E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B8"), 0x45, 0x3);
	registerAssetPools(hook::pattern("48 8D 15 ? ? ? ? C6 40 E8 ? 41 B9 ? ? ? ? C6"), 0x23, 0x3);

	// no-op assertation to ensure our pool crash reporting is used instead
	hook::nop(hook::get_pattern("83 C9 FF BA EF 4F 91 02 E8", 8), 5);

	//get the initial pools
	if (GetModuleHandle("ScriptHookRDR2.dll") != nullptr) //If using SHV use different approach to avoid double hook
	{
		auto addr = hook::get_address<uint8_t*>(hook::get_module_pattern<uint8_t>("ScriptHookRDR2.dll", "BA 10 00 00 00 48 8B CB", -0x9)); //address of ScriptHookRDR2 Detour function

		MH_CreateHook(addr, GetSizeOfPool, (void**)&g_origSizeOfPool);
	}
	//ScriptHookRDR2.dll is not present hook into original rage::fwConfigManager::GetSizeOfPool inside the executable.
	else
	{
		auto loc = hook::get_call(hook::get_pattern<uint8_t>("BA 95 ? ? ? 41 B8 B8 0B ? ?", 0xB));  // get the address of originial function form jmp 0x41663795 -- maxloadrequestedinfo

		MH_CreateHook(loc, GetSizeOfPool, (void**)&g_origSizeOfPool);
	}

	MH_CreateHook(hook::get_pattern<uint8_t>("4C 63 41 1C 4C 8B D1 49 3B D0 76", -4), PoolAllocateWrap, (void**)&g_origPoolAllocate);
	MH_CreateHook(hook::get_pattern<uint8_t>("8B 41 28 A9 00 00 00 C0 74", -15), PoolDtorWrap, (void**)&g_origPoolDtor);

	MH_EnableHook(MH_ALL_HOOKS);
}


BOOL WINAPI DllMain(_In_ void* _DllHandle, _In_ unsigned long _Reason, _In_opt_ void* _Reserved)
{
	switch (_Reason)
	{
	case  DLL_PROCESS_ATTACH:
		//	MessageBoxA(nullptr, "PoolManager", "Test", MB_OK); //used for debugging
		InitializeMod();
		break;
	}
	return TRUE;
}