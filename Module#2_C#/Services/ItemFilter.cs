using DndItemGenerator.Models;

namespace DndItemGenerator.Services
{
    public static class ItemFilter
    {
        public static List<MagicItem> Filter(
            IEnumerable<MagicItem> items,
            string? selectedRarity,
            string? selectedType,
            string? selectedSource)
        {
            IEnumerable<MagicItem> result = items;

            // RARITY
            if (!string.IsNullOrWhiteSpace(selectedRarity) &&
                !selectedRarity.Equals(
                    "Any",
                    StringComparison.OrdinalIgnoreCase))
            {
                result = result.Where(item =>
                    item.Rarity.Equals(
                        selectedRarity,
                        StringComparison.OrdinalIgnoreCase));
            }

            // TYPE
            if (!string.IsNullOrWhiteSpace(selectedType) &&
                !selectedType.Equals(
                    "Any",
                    StringComparison.OrdinalIgnoreCase))
            {
                result = result.Where(item =>
                    TypeMatches(
                        item.Type,
                        selectedType));
            }

            // SOURCE
            if (!string.IsNullOrWhiteSpace(selectedSource) &&
                !selectedSource.Equals(
                    "Any",
                    StringComparison.OrdinalIgnoreCase))
            {
                result = result.Where(item =>
                    item.Source.Equals(
                        selectedSource,
                        StringComparison.OrdinalIgnoreCase));
            }

            return result.ToList();
        }

        private static bool TypeMatches(
            string itemType,
            string selectedType)
        {
            // Selecting "Weapon" also includes:
            // Weapon
            // Weapon (dagger)
            // Weapon (sword)
            // etc. 
            if (selectedType.Equals(
                    "Weapon",
                    StringComparison.OrdinalIgnoreCase))
            {
                return itemType.StartsWith(
                    "Weapon",
                    StringComparison.OrdinalIgnoreCase);
            }

            if (selectedType.Equals(
                    "Armor",
                    StringComparison.OrdinalIgnoreCase))
            {
                return itemType.StartsWith(
                    "Armor",
                    StringComparison.OrdinalIgnoreCase);
            }

            return itemType.Equals(
                selectedType,
                StringComparison.OrdinalIgnoreCase);
        }
    }
}