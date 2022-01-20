#include "atPool.h"
#include <Hooking.h>
#include <MinHook.h>
#include <iostream>
#include <Utils.h>
#include <sstream>
#include <iostream>
#include <fstream>

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char* (&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ HashString(list[i]), list[i] });
		}
	}

	inline std::string LookupHashString(uint32_t hash)
	{
		if (hash == NULL)
		{
			return "Unknown";
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
			return "Unknown";
		}

		char buffer[32];
		snprintf(buffer, std::size(buffer), "0x%08X", hash);
		return buffer;
	}

private:
	std::map<uint32_t, std::string_view> m_lookupList;
};

int LogAllPoolCalls = GetPrivateProfileInt("POOL_SETTINGS", "LogAllPoolCalls", 0, ".\\PoolManager.ini");
int LogStartupPoolCalls = GetPrivateProfileInt("POOL_SETTINGS", "LogStartupPoolCalls ", 0, ".\\PoolManager.ini");

//a boolean flag that lets us "remember" if this thing has happened already
bool clearedLogs = false;

void cleanUpLogs()
{
	if (!clearedLogs)
	{
		if (LogAllPoolCalls != 0)
		{
			std::ofstream outfile;
			outfile.open("PoolManager_Verbose.log", std::ofstream::out | std::ofstream::trunc);
			outfile.close();
		}

		if (LogStartupPoolCalls != 0)
		{
			std::ofstream outfile;
			outfile.open("PoolManager_Startup.log", std::ofstream::out | std::ofstream::trunc);
			outfile.close();
		}

		clearedLogs = true;
	}
}


static std::map<uint32_t, atPoolBase*> g_pools;
static std::map<atPoolBase*, uint32_t> g_inversePools;

static const char* poolEntriesTable[] = {
	"AnimatedBuilding",
	"AttachmentExtension",
	"AudioHeap",
	"BlendshapeStore",
	"Building",
	"carrec",
	"CBoatChaseDirector",
	"CVehicleCombatAvoidanceArea",
	"CCargen",
	"CCargenForScenarios",
	"CCombatDirector",
	"CCombatInfo",
	"CCombatSituation",
	"CCoverFinder",
	"CDefaultCrimeInfo",
	"CTacticalAnalysis",
	"CTaskUseScenarioEntityExtension",
	"AnimStore",
	"CGameScriptResource",
	"ClothStore",
	"CombatMeleeManager_Groups",
	"CombatMountedManager_Attacks",
	"CompEntity",
	"CPrioritizedClipSetBucket",
	"CPrioritizedClipSetRequest",
	"CRoadBlock",
	"CStuntJump",
	"CScenarioInfo",
	"CScenarioPointExtraData",
	"CutsceneStore",
	"CScriptEntityExtension",
	"CVehicleChaseDirector",
	"CVehicleClipRequestHelper",
	"CPathNodeRouteSearchHelper",
	"CGrabHelper",
	"CGpsNumNodesStored",
	"CClimbHandHoldDetected",
	"CAmbientLookAt",
	"DecoratorExtension",
	"DrawableStore",
	"Dummy Object",
	"DwdStore",
	"EntityBatch",
	"GrassBatch",
	"ExprDictStore",
	"FrameFilterStore",
	"FragmentStore",
	"GamePlayerBroadcastDataHandler_Remote",
	"InstanceBuffer",
	"InteriorInst",
	"InteriorProxy",
	"IplStore",
	"MaxLoadedInfo",
	"MaxLoadRequestedInfo",
	"ActiveLoadedInfo",
	"ActivePersistentLoadedInfo",
	"Known Refs",
	"LightEntity",
	"MapDataLoadedNode",
	"MapDataStore",
	"MapTypesStore",
	"MetaDataStore",
	"NavMeshes",
	"NetworkDefStore",
	"NetworkCrewDataMgr",
	"Object",
	"OcclusionInteriorInfo",
	"OcclusionPathNode",
	"OcclusionPortalEntity",
	"OcclusionPortalInfo",
	"Peds",
	"CWeapon",
	"phInstGta",
	"PhysicsBounds",
	"CPickup",
	"CPickupPlacement",
	"CPickupPlacementCustomScriptData",
	"CRegenerationInfo",
	"PortalInst",
	"PoseMatcherStore",
	"PMStore",
	"PtFxSortedEntity",
	"PtFxAssetStore",
	"QuadTreeNodes",
	"ScaleformStore",
	"ScaleformMgrArray",
	"ScriptStore",
	"StaticBounds",
	"tcBox",
	"TrafficLightInfos",
	"TxdStore",
	"Vehicles",
	"VehicleStreamRequest",
	"VehicleStreamRender",
	"VehicleStruct",
	"HandlingData",
	"wptrec",
	"fwLodNode",
	"CTask",
	"CEvent",
	"CMoveObject",
	"CMoveAnimatedBuilding",
	"atDScriptObjectNode",
	"fwDynamicArchetypeComponent",
	"fwDynamicEntityComponent",
	"fwEntityContainer",
	"fwMatrixTransform",
	"fwQuaternionTransform",
	"fwSimpleTransform",
	"ScenarioCarGensPerRegion",
	"ScenarioPointsAndEdgesPerRegion",
	"ScenarioPoint",
	"ScenarioPointEntity",
	"ScenarioPointWorld",
	"MaxNonRegionScenarioPointSpatialObjects",
	"ObjectIntelligence",
	"VehicleScenarioAttractors",
	"AircraftFlames",
	"CScenarioPointChainUseInfo",
	"CScenarioClusterSpawnedTrackingData",
	"CSPClusterFSMWrapper",
	"fwArchetypePooledMap",
	"CTaskConversationHelper",
	"SyncedScenes",
	"AnimScenes",
	"CPropManagementHelper",
	"ActionTable_Definitions",
	"ActionTable_Results",
	"ActionTable_Impulses",
	"ActionTable_Interrelations",
	"ActionTable_Homings",
	"ActionTable_Damages",
	"ActionTable_StrikeBones",
	"ActionTable_Rumbles",
	"ActionTable_Branches",
	"ActionTable_StealthKills",
	"ActionTable_Vfx",
	"ActionTable_FacialAnimSets",
	"NetworkEntityAreas",
	"NavMeshRoute",
	"CScriptEntityExtension",
	"AnimStore",
	"CutSceneStore",
	"OcclusionPathNode",
	"OcclusionPortalInfo",
	"CTask",
	"OcclusionPathNode",
	"OcclusionPortalInfo",
};

static RageHashList poolEntries(poolEntriesTable);

static atPoolBase* SetPoolFn(atPoolBase* pool, uint32_t hash)
{
	g_pools[hash] = pool;
	g_inversePools.insert({ pool, hash });

	if (LogStartupPoolCalls != 0)
	{
		cleanUpLogs();
		std::string poolName = poolEntries.LookupHashString(hash);
		std::string poolNameHash = poolEntries.LookupHash(hash);

		std::ofstream outfile;
		outfile.open("PoolManager_Startup.log", std::ios_base::app);
		outfile << "poolName: " << poolName.c_str() << std::endl
			<< "poolHash: " << poolNameHash.c_str() << std::endl
			<< "poolSize: " << pool->GetSize() << std::endl
			<< std::endl;
	}

	return pool;
}

static void(*g_origPoolDtor)(atPoolBase*);

static void PoolDtorWrap(atPoolBase* pool)
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
static void* (*g_origPoolAllocate)(atPoolBase*, uint64_t);

static void* PoolAllocateWrap(atPoolBase* pool, uint64_t unk)
{
	void* value = g_origPoolAllocate(pool, unk);


	if (LogAllPoolCalls != 0)
	{
		cleanUpLogs();
		auto it = g_inversePools.find(pool);
		uint32_t poolHash = it->second;
		std::string poolName = poolEntries.LookupHashString(poolHash);
		std::string poolNameHash = poolEntries.LookupHash(poolHash);

		std::ofstream outfile;
		outfile.open("PoolManager_Verbose.log", std::ios_base::app);
		outfile << "poolName: " << poolName.c_str() << std::endl
			<< "poolHash: " << poolNameHash.c_str() << std::endl
			<< "poolSize: " << pool->GetSize() << std::endl
			<< "poolCount: " << pool->GetCount() << std::endl
			<< std::endl;
	}

	if (!value)
	{
		auto it = g_inversePools.find(pool);

		uint32_t poolHash = it->second;
		std::string poolName = poolEntries.LookupHashString(poolHash);
		std::string poolNameHash = poolEntries.LookupHash(poolHash);

		std::ofstream outfile;
		outfile.open("PoolManager_Crash.log", std::ios_base::app);
		outfile << "poolName: " << poolName.c_str() << std::endl
			<< "poolHash: " << poolNameHash.c_str() << std::endl
			<< "poolSize: " << pool->GetSize() << std::endl
			<< std::endl;
		outfile.close();

		char buff[256];
		std::string extraWarning;
		if (poolName.find("0x") == std::string::npos)
		{
			sprintf_s(buff, "\nYou need to raise %s's pool size in update.rpf/common/data/gameconfig.xml", poolName.c_str());
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


static struct MhInit
{
	MhInit()
	{
		MH_Initialize();
	}
} mhInit;


void InitializeMod()
{

	auto registerPools = [](hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			auto hash = *match.get<uint32_t>(hashOffset);

			struct : jitasm::Frontend
			{
				uint32_t hash;
				uint64_t origFn;

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
					mov(edx, hash);

					mov(rax, (uint64_t)&SetPoolFn);
					call(rax);

					add(rsp, 0x38);

					ret();
				}
			}*stub = new std::remove_pointer_t<decltype(stub)>();

			stub->hash = hash;

			auto call = match.get<void>(callOffset);
			hook::set_call(&stub->origFn, call);
			hook::call(call, stub->GetCode());
		}
	};

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 8B D8 E8"), 51, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CD"), 41, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CE"), 45, 1);

	// no-op assertation to ensure our pool crash reporting is used instead
	hook::nop(hook::get_pattern("83 C9 FF BA EF 4F 91 02 E8", 8), 5);

	MH_CreateHook(hook::get_pattern("4C 63 41 1C 4C 8B D1 49 3B D0 76", -4), PoolAllocateWrap, (void**)&g_origPoolAllocate);
	MH_CreateHook(hook::get_pattern("8B 41 28 A9 00 00 00 C0 74", -15), PoolDtorWrap, (void**)&g_origPoolDtor);

	MH_EnableHook(MH_ALL_HOOKS);

}

BOOL WINAPI DllMain(_In_ void* _DllHandle, _In_ unsigned long _Reason, _In_opt_ void* _Reserved)
{
	if (_Reason == DLL_PROCESS_ATTACH)
	{
		InitializeMod();
	}
	return TRUE;
}