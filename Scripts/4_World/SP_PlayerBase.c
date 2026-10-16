modded class PlayerBase
{
    protected bool m_SP_Routing;

    bool SP_IsRouting()
    {
        return m_SP_Routing;
    }

    bool SP_TakePickup(EntityAI entity)
    {
        ItemBase item = ItemBase.Cast(entity);
        if (m_SP_Routing || !item || !SP_Pickup.CanPickUp(this, item))
            return false;
        m_SP_Routing = true;
        bool result = SP_RoutePickup(item);
        m_SP_Routing = false;
        return result;
    }

    protected bool SP_RoutePickup(ItemBase item)
    {
        InventoryLocation destination;
        if (SP_Pickup.FindAttachment(this, item, destination))
        {
            // Preserve vanilla attachment handling, including the weapon manager.
            return super.PredictiveTakeEntityToTargetAttachmentEx(destination.GetParent(), item, destination.GetSlot());
        }
        if (SP_Pickup.FindStack(this, item))
        {
            if (GetGame().IsMultiplayer())
            {
                if (!GetGame().IsClient() || GetGame().GetPlayer() != this)
                    return false;
                GetGame().RPCSingleParam(this, SP_Pickup.RPC_STACK_PICKUP, new Param1<ItemBase>(item), true);
            }
            else
            {
                SP_Pickup.StackPickupServer(this, item);
            }
            return true;
        }
        return super.PredictiveTakeEntityToInventory(FindInventoryLocationType.CARGO, item);
    }

    override bool PredictiveTakeEntityToInventory(FindInventoryLocationType flags, notnull EntityAI item)
    {
        if (!m_SP_Routing && (flags & FindInventoryLocationType.ANY_CARGO) && SP_Pickup.IsIncoming(this, item))
            return SP_TakePickup(item);
        return super.PredictiveTakeEntityToInventory(flags, item);
    }

    override bool PredictiveTakeEntityToTargetInventory(notnull EntityAI target, FindInventoryLocationType flags, notnull EntityAI item)
    {
        if (!m_SP_Routing && (flags & FindInventoryLocationType.ANY_CARGO) && SP_Pickup.IsOwned(this, target) && SP_Pickup.IsIncoming(this, item))
            return SP_TakePickup(item);
        return super.PredictiveTakeEntityToTargetInventory(target, flags, item);
    }

    override bool PredictiveTakeEntityToTargetCargo(notnull EntityAI target, notnull EntityAI item)
    {
        if (!m_SP_Routing && SP_Pickup.IsOwned(this, target) && SP_Pickup.IsIncoming(this, item))
            return SP_TakePickup(item);
        return super.PredictiveTakeEntityToTargetCargo(target, item);
    }

    override bool PredictiveTakeEntityToTargetCargoEx(notnull CargoBase cargo, notnull EntityAI item, int row, int col)
    {
        if (!m_SP_Routing && SP_Pickup.IsOwned(this, cargo.GetCargoOwner()) && SP_Pickup.IsIncoming(this, item))
            return SP_TakePickup(item);
        return super.PredictiveTakeEntityToTargetCargoEx(cargo, item, row, col);
    }

    override bool PredictiveTakeToDst(notnull InventoryLocation src, notnull InventoryLocation dst)
    {
        int kind = dst.GetType();
        if (!m_SP_Routing && (kind == InventoryLocationType.CARGO || kind == InventoryLocationType.ATTACHMENT) && SP_Pickup.IsOwned(this, dst.GetParent()) && SP_Pickup.IsIncoming(this, src.GetItem()))
            return SP_TakePickup(src.GetItem());
        return super.PredictiveTakeToDst(src, dst);
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        if (rpc_type != SP_Pickup.RPC_STACK_PICKUP)
        {
            super.OnRPC(sender, rpc_type, ctx);
            return;
        }
        // Authenticate the actual network sender against this player instance.
        if (!GetGame().IsServer() || !sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
            return;
        Param1<ItemBase> request;
        if (ctx.Read(request) && request && request.param1)
            SP_Pickup.StackPickupServer(this, request.param1);
    }
}


