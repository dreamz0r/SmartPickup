modded class ActionTakeItem
{
    // Keep the native condition first; add only the missing full-inventory attachment or merge case.
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (super.ActionCondition(player, target, item))
            return true;
        if (!player || !target)
            return false;
        ItemBase pickedItem = ItemBase.Cast(target.GetObject());
        if (!pickedItem || player.GetInventory().CanAddEntityIntoInventory(pickedItem))
            return false;
        if (!SP_Pickup.HasPickupSpace(player, pickedItem))
            return false;

        // Match vanilla's parent, attachment and code-lock restrictions in this extra case.
        EntityAI parent = EntityAI.Cast(target.GetParent());
        if (parent && (!parent.CanUseConstruction() || !pickedItem.CanDetachAttachment(parent) || !parent.CanReleaseAttachment(pickedItem)))
            return false;
        DigitalCodeLock codeLock = DigitalCodeLock.Cast(pickedItem);
        if (codeLock && !codeLock.CanDetachCodeLock(player, target.GetComponentIndex()))
            return false;
        return true;
    }

    override bool InventoryReservation(ActionData action_data)
    {
        PlayerBase player = action_data.m_Player;
        ItemBase item = ItemBase.Cast(action_data.m_Target.GetObject());
        if (!item || !SP_Pickup.CanPickUp(player, item))
            return false;

        InventoryLocation reservation;
        if (!SP_Pickup.FindAttachment(player, item, reservation))
        {
            reservation = new InventoryLocation;
            if (SP_Pickup.FindStack(player, item))
            {
                // Reserve the source when a merge needs no free destination cell.
                if (!item.GetInventory().GetCurrentInventoryLocation(reservation))
                    return false;
            }
            else if (!player.GetInventory().FindFreeLocationFor(item, FindInventoryLocationType.CARGO, reservation))
            {
                return false;
            }
        }
        if (player.GetInventory().HasInventoryReservation(item, reservation))
            return false;
        if (!player.GetInventory().AddInventoryReservationEx(item, reservation, GameInventory.c_InventoryReservationTimeoutMS))
            return false;
        action_data.m_ReservedInventoryLocations.Insert(reservation);
        return true;
    }

    override void OnExecute(ActionData action_data)
    {
        if (GetGame().IsDedicatedServer())
        {
            ClearActionJuncture(action_data);
            return;
        }
        ItemBase item = ItemBase.Cast(action_data.m_Target.GetObject());
        ClearInventoryReservationEx(action_data);
        if (item)
            action_data.m_Player.SP_TakePickup(item);
    }
}


