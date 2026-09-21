namespace DndItemGenerator.Models
{
    public class MagicItem // this is a class representing one dnd item
    {
        //all of these come from the csv file structure.
        public string Name {get; set; } = "";
        public string Type {get; set; } = "";
        public string Attunement {get; set; } = "";
        public string Price {get; set; } = "";
        public string Source {get; set; } = "";
        public string Damage {get; set; } = "";
        public string Description {get; set; } = "";

        //comes from the file name of the CSV
        public string Rarity {get; set;} = "";
    }
}