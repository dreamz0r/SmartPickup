modded class ItemBase
{
    override void SplitIntoStackMaxClient(EntityAI destination_entity, int slot_id)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player && !player.SP_IsRouting() && SP_Pickup.IsOwned(player, destination_entity) && SP_Pickup.IsIncoming(player, this))
        {
            player.SP_TakePickup(this);
            return;
        }
        super.SplitIntoStackMaxClient(destination_entity, slot_id);
    }

    override void SplitIntoStackMaxToInventoryLocationClient(notnull InventoryLocation dst)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        int kind = dst.GetType();
        if (player && !player.SP_IsRouting() && (kind == InventoryLocationType.CARGO || kind == InventoryLocationType.ATTACHMENT) && SP_Pickup.IsOwned(player, dst.GetParent()) && SP_Pickup.IsIncoming(player, this))
        {
            player.SP_TakePickup(this);
            return;
        }
        super.SplitIntoStackMaxToInventoryLocationClient(dst);
    }
}

