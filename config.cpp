class CfgPatches
{
    class SmartPickup
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};
class CfgMods
{
    class SmartPickup
    {
        dir = "SmartPickup";
        name = "Smart Pickup";
        author = "Griffin";
        type = "mod";
        dependencies[] = {"World", "Mission"};
        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"SmartPickup/Scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"SmartPickup/Scripts/5_Mission"};
            };
        };
    };
};


