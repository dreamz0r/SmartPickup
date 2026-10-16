// One scan per pickup; no timers, background sorting, settings, or dependencies.
class SP_Pickup
{
    static const int RPC_STACK_PICKUP = 1749628301;

    static bool IsOwned(PlayerBase player, EntityAI entity)
    {
        return entity && (entity == player || entity.GetHierarchyRootPlayer() == player);
    }

    static bool IsIncoming(PlayerBase player, EntityAI entity)
    {
        return player && entity && !IsOwned(player, entity);
    }

    static bool CanPickUp(PlayerBase player, ItemBase item)
    {
        if (!IsIncoming(player, item) || !player.IsAlive() || player.IsUnconscious())
            return false;
        if (player.IsRestrained() || player.IsRestrainPrelocked() || player.IsRestrainStarted())
            return false;
        if (!player.CanManipulateInventory() || player.IsItemsToDelete() || player.GetInventory().IsInventoryLocked())
            return false;
        if (!item.IsTakeable() || item.IsBeingPlaced() || item.IsHologram() || item.IsSetForDeletion())
            return false;
        if (!item.GetInventory().CanRemoveEntity())
            return false;

        InventoryLocation source = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(source))
            return false;
        return player.GetDayZPlayerInventory().PlayerCheckRequestSrc(source, GameInventory.c_MaxItemDistanceRadius);
    }

    static float Amount(ItemBase item)
    {
        if (!item || item.IsSetForDeletion())
            return 0;
        Magazine magazine = Magazine.Cast(item);
        if (magazine)
            return magazine.GetAmmoCount();
        return item.GetQuantity();
    }

    static bool IsEquippedOwner(PlayerBase player, EntityAI owner)
    {
        while (owner && owner != player)
        {
            InventoryLocation current = new InventoryLocation;
            if (!owner.GetInventory().GetCurrentInventoryLocation(current))
                return false;
            int kind = current.GetType();
            if (kind != InventoryLocationType.ATTACHMENT && kind != InventoryLocationType.HANDS)
                return false;
            owner = current.GetParent();
        }
        return owner == player;
    }

    static InventoryLocation FindAttachmentOnOwner(PlayerBase player, EntityAI owner, ItemBase item, InventoryLocation source)
    {
        if (!IsOwned(player, owner) || owner == item)
            return null;
        GameInventory ownerInventory = owner.GetInventory();
        if (ownerInventory.IsInventoryLocked())
            return null;

        // Keep magazine insertion under the normal weapon-manager restrictions.
        Weapon_Base weapon = Weapon_Base.Cast(owner);
        Magazine magazine = Magazine.Cast(item);
        if (weapon && magazine && !player.GetWeaponManager().CanAttachMagazine(weapon, magazine))
            return null;

        GameInventory itemInventory = item.GetInventory();
        for (int index = 0; index < itemInventory.GetSlotIdCount(); index++)
        {
            int slot = itemInventory.GetSlotId(index);
            if (!ownerInventory.HasAttachmentSlot(slot) || ownerInventory.GetSlotLock(slot) || ownerInventory.FindAttachment(slot))
                continue;
            if (!ownerInventory.CanAddAttachmentEx(item, slot))
                continue;
            InventoryLocation location = new InventoryLocation;
            location.SetAttachment(owner, item, slot);
            if (player.GetDayZPlayerInventory().PlayerCheckRequestDst(source, location, GameInventory.c_MaxItemDistanceRadius))
                return location;
        }
        return null;
    }

    static bool FindAttachment(PlayerBase player, ItemBase item, out InventoryLocation destination)
    {
        InventoryLocation source = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(source))
            return false;

        // Check the player first, then the actual slots on each piece of gear.
        InventoryLocation location = FindAttachmentOnOwner(player, player, item, source);
        if (location)
        {
            destination = location;
            return true;
        }
        array<EntityAI> owners = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, owners);
        for (int pass = 0; pass < 2; pass++)
        {
            foreach (EntityAI owner : owners)
            {
                if (owner == player || !IsOwned(player, owner))
                    continue;
                bool equipped = IsEquippedOwner(player, owner);
                if ((pass == 0 && !equipped) || (pass == 1 && equipped))
                    continue;
                location = FindAttachmentOnOwner(player, owner, item, source);
                if (location)
                {
                    destination = location;
                    return true;
                }
            }
        }
        return false;
    }
    static bool CanStackInto(PlayerBase player, ItemBase source, ItemBase stack)
    {
        if (!stack || stack == source || !IsOwned(player, stack) || stack.IsSetForDeletion() || stack.IsHologram())
            return false;
        if (Amount(source) <= 0 || !stack.CanBeCombined(source, true))
            return false;
        InventoryLocation location = new InventoryLocation;
        if (!stack.GetInventory().GetCurrentInventoryLocation(location))
            return false;
        return player.GetDayZPlayerInventory().PlayerCheckRequestSrc(location, GameInventory.c_MaxItemDistanceRadius);
    }

    static ItemBase FindStack(PlayerBase player, ItemBase item)
    {
        array<EntityAI> contents = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, contents);
        foreach (EntityAI entity : contents)
        {
            ItemBase stack = ItemBase.Cast(entity);
            if (CanStackInto(player, item, stack))
                return stack;
        }
        return null;
    }

    // Extend UI capacity checks only for external pickups into the player's inventory.
    static bool HasPickupSpace(PlayerBase player, EntityAI entity)
    {
        ItemBase item = ItemBase.Cast(entity);
        if (!item || !CanPickUp(player, item))
            return false;
        InventoryLocation attachment;
        return FindAttachment(player, item, attachment) || FindStack(player, item) != null;
    }

    static bool MoveRemainder(PlayerBase player, ItemBase item, InventoryLocation destination)
    {
        InventoryLocation source = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(source))
            return false;
        if (!player.GetDayZPlayerInventory().PlayerCheckRequestDst(source, destination, GameInventory.c_MaxItemDistanceRadius))
            return false;
        float maximum = item.GetTargetQuantityMax(destination.GetSlot());
        if (maximum > 0 && maximum < item.GetQuantity())
        {
            // Keep vanilla slot-specific stack limits and property transfer.
            return item.SplitIntoStackMaxToInventoryLocationEx(destination) != null;
        }
        return player.ServerTakeToDst(source, destination);
    }

    // Only called by the server (or the local single-player authority).
    static void StackPickupServer(PlayerBase player, ItemBase item)
    {
        if (!GetGame().IsServer() || !CanPickUp(player, item))
            return;

        // Recheck priority: a slot may have become free since the request was sent.
        InventoryLocation destination;
        if (FindAttachment(player, item, destination))
        {
            MoveRemainder(player, item, destination);
            return;
        }

        array<EntityAI> contents = new array<EntityAI>;
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, contents);
        foreach (EntityAI entity : contents)
        {
            if (!item || Amount(item) <= 0)
                return;
            ItemBase stack = ItemBase.Cast(entity);
            if (CanStackInto(player, item, stack))
            {
                // Vanilla combines preserve item rules, health, agents and real cartridges.
                stack.CombineItems(item, true);
            }
        }

        if (!item || Amount(item) <= 0 || !CanPickUp(player, item))
            return;
        destination = new InventoryLocation;
        if (player.GetInventory().FindFreeLocationFor(item, FindInventoryLocationType.CARGO, destination))
            MoveRemainder(player, item, destination);
        // A partial stack stays in its original location if cargo is full.
    }
}

