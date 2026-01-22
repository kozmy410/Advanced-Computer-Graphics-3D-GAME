#pragma once
#include <string>
#include <vector>
#include <iostream>

// defining what a single slot in the backpack looks like
struct InventoryItem {
    std::string name;       // what the item actually is, e.g., "Apple"
    int quantity = 0;       // how many we have right now
    int maxStack = 64;      // max items we can shove in one pile

    // checking if there's actually anything in here
    bool isEmpty() const { return quantity <= 0; }
};

class Inventory {
public:
    // the actual list of pockets/slots
    std::vector<InventoryItem> slots;

    // which slot the player is holding right now (for the hotbar)
    int selectedSlot = 0;

    // constructor: setting up the backpack size
    Inventory(int size) {
        slots.resize(size);
    }

    // trying to pick something up. returns true if we actually grabbed it
    bool addItem(const std::string& name, int count) {
        // 1. first, looking for the same item to stack onto
        for (auto& item : slots) {
            // if we found a matching pile that isn't full yet
            if (!item.isEmpty() && item.name == name && item.quantity < item.maxStack) {
                int space = item.maxStack - item.quantity;

                // only add what fits in this specific slot
                int toAdd = (count < space) ? count : space;

                item.quantity += toAdd;
                count -= toAdd;

                // if we managed to stow everything away, we're done
                if (count == 0) return true;
            }
        }

        // 2. if we still have stuff to add, look for the first empty slot
        if (count > 0) {
            for (auto& item : slots) {
                if (item.isEmpty()) {
                    // found an empty spot, putting it here
                    item.name = name;
                    item.quantity = count;
                    return true;
                }
            }
        }

        // if we get here, the backpack is stuffed
        std::cout << "Inventory Full!" << std::endl;
        return false;
    }

    // tossing something out or using it
    void removeItem(int slot, int count) {
        // making sure the slot exists and has stuff in it
        if (slot >= 0 && slot < slots.size() && !slots[slot].isEmpty()) {
            slots[slot].quantity -= count;

            // if we used the last one, clear the name so it's officially empty
            if (slots[slot].quantity <= 0) {
                slots[slot].quantity = 0;
                slots[slot].name = "";
            }
        }
    }
};