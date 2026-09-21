using DndItemGenerator.Models;

namespace DndItemGenerator.Services
{
    public static class RollTableBuilder
    {
        public static List<RollTableEntry> Build(
            IEnumerable<MagicItem> sourceItems,
            bool randomize = false)
        {
            List<MagicItem> items = sourceItems.ToList();//copy the magic items

            if (randomize)
            {
                Shuffle(items);
            }

            List<RollTableEntry> table = new();

            for (int i = 0; i < items.Count; i++)//assign each item a number
            {
                table.Add(new RollTableEntry
                {
                    RollNumber = i + 1,//start at 1 not 0 which is the regular way to count as dice start at 1
                    Item = items[i]
                });
            }

            return table;
        }
        //Table Randomizer
        private static void Shuffle<T>(IList<T> list) //reassign each item a new number
        {
            for (int i = list.Count - 1; i > 0; i--)
            {
                int j = Random.Shared.Next(i + 1);

                (list[i], list[j]) =
                    (list[j], list[i]);
            }
        }
    }
}