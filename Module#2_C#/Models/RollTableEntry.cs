namespace DndItemGenerator.Models
{
    public class RollTableEntry
    {
        public int RollNumber { get; set; }

        public MagicItem Item { get; set; } = new MagicItem();
    }
}