// Vanilla double-click code sometimes demands a free cell before trying a pickup.
class SP_InventoryPickup
{
    static bool TryItem(EntityAI entity)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !SP_Pickup.IsIncoming(player, entity))
            return false;
        if (player.GetInventory().HasInventoryReservation(entity, null))
            return false;
        return player.SP_TakePickup(entity);
    }

    static bool TryDoubleClick(Widget widget, int button)
    {
        if (!widget || button != MouseState.LEFT || GetGame().IsLeftCtrlDown())
            return false;
        ItemPreviewWidget preview = ItemPreviewWidget.Cast(widget.FindAnyWidget("Render"));
        if (!preview)
        {
            string name = widget.GetName();
            name.Replace("PanelWidget", "Render");
            preview = ItemPreviewWidget.Cast(widget.FindAnyWidget(name));
        }
        if (!preview)
            preview = ItemPreviewWidget.Cast(widget);
        if (!preview)
            return false;
        SlotsIcon icon;
        widget.GetUserData(icon);
        if (!icon)
            preview.GetUserData(icon);
        if (icon && (icon.IsReserved() || icon.IsOutOfReach()))
            return false;
        return TryItem(preview.GetItem());
    }
}

modded class Icon
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (!m_HandsIcon && SP_InventoryPickup.TryDoubleClick(w, button))
        {
            RefreshQuickbar();
            return;
        }
        super.DoubleClick(w, x, y, button);
    }
}

// The inventory's left area delegates keyboard/controller transfers through here.
modded class Container
{
    override bool CanTakeToInventoryEx(EntityAI focusedEntity)
    {
        if (super.CanTakeToInventoryEx(focusedEntity))
            return true;
        if (ItemManager.GetInstance().IsMicromanagmentMode())
            return false;
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        return player && SP_Pickup.HasPickupSpace(player, focusedEntity);
    }
    override bool TransferItem()
    {
        if (!ItemManager.GetInstance().IsMicromanagmentMode() && SP_InventoryPickup.TryItem(GetFocusedItem()))
            return true;
        return super.TransferItem();
    }
}

modded class VicinitySlotsContainer
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (SP_InventoryPickup.TryDoubleClick(w, button))
            return;
        super.DoubleClick(w, x, y, button);
    }
}


modded class ContainerWithCargoAndAttachments
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (SP_InventoryPickup.TryDoubleClick(w, button))
            return;
        super.DoubleClick(w, x, y, button);
    }
}


modded class PlayerContainer
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (SP_InventoryPickup.TryDoubleClick(w, button))
            return;
        super.DoubleClick(w, x, y, button);
    }
}


modded class ZombieContainer
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (SP_InventoryPickup.TryDoubleClick(w, button))
            return;
        super.DoubleClick(w, x, y, button);
    }
}


modded class AttachmentCategoriesRow
{
    override void DoubleClick(Widget w, int x, int y, int button)
    {
        if (SP_InventoryPickup.TryDoubleClick(w, button))
            return;
        super.DoubleClick(w, x, y, button);
    }
}



