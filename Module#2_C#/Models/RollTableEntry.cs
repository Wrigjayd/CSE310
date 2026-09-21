namespace DndItemGenerator.Models
{
    //pairs the magic item to the rolled number
    public class RollTableEntry
    {
        public int RollNumber { get; set; }

        public MagicItem Item { get; set; } = new MagicItem();
    }
}