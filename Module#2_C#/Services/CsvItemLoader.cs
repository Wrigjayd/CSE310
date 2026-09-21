using System.IO;
using Microsoft.VisualBasic.FileIO;
using DndItemGenerator.Models;

namespace DndItemGenerator.Services
{
    public static class CsvItemLoader
    {
        public static List<MagicItem> LoadFile(string filePath)//load the files to read
        {
            List<MagicItem> items = new();

            string rarity = GetRarityFromFileName(filePath);

            using TextFieldParser parser = new(filePath);

            parser.TextFieldType = FieldType.Delimited;
            parser.SetDelimiters(",");
            parser.HasFieldsEnclosedInQuotes = true;

            if (parser.EndOfData)
                return items;

            string[]? headers = parser.ReadFields(); //reading headers

            if (headers == null)
                return items;

            Dictionary<string, int> columnIndexes =
                new(StringComparer.OrdinalIgnoreCase);

            // Build the header dictionary safely without breaking itself.
            for (int i = 0; i < headers.Length; i++)
            {
                string header = headers[i].Trim();

                // Ignore completely blank column names.
                if (string.IsNullOrWhiteSpace(header))
                    continue;

                // Ignore duplicate column headers.
                if (!columnIndexes.ContainsKey(header))
                {
                    columnIndexes.Add(header, i);
                }
            }

            while (!parser.EndOfData) //read the data
            {
                string[]? fields;

                try
                {
                    fields = parser.ReadFields();
                }
                catch (MalformedLineException)
                {
                    // Skip malformed CSV rows instead of
                    // crashing the entire program.
                    continue;
                }

                if (fields == null)
                    continue;

                string GetValue(string columnName)//looks for certain names in the dictionaries
                {
                    if (!columnIndexes.TryGetValue(
                            columnName,
                            out int index))
                    {
                        return "";
                    }

                    if (index >= fields.Length)
                        return "";

                    return fields[index]?.Trim() ?? "";
                }
                //calling magic item class and creating the magic items
                MagicItem item = new()
                {
                    Name = GetValue("Name"),
                    Type = GetValue("Type"),
                    Attunement = GetValue("Attunement"),
                    Price = GetValue("Price"),
                    Source = GetValue("Source"),
                    Damage = GetValue("Damage"),
                    Description = GetValue("Description"),
                    Rarity = rarity
                };

                if (!string.IsNullOrWhiteSpace(item.Name))
                {
                    items.Add(item);
                }//prevents black rows from being items
            }

            return items;
        }
        //read out the csv data into a list
        public static List<MagicItem> LoadFolder(//open all the csvs at once
            string folderPath)
        {
            List<MagicItem> allItems = new();

            if (!Directory.Exists(folderPath))
                return allItems;

            string[] csvFiles =
                Directory.GetFiles(
                    folderPath,
                    "*.csv");

            foreach (string file in csvFiles)
            {
                List<MagicItem> fileItems =
                    LoadFile(file);

                allItems.AddRange(fileItems);
            }

            return allItems;
        }

        private static string GetRarityFromFileName(//for searching by rarity get the rarity from the file name
            string filePath)
        {
            string fileName =
                Path.GetFileNameWithoutExtension(
                    filePath);

            // Very Rare must come before Rare.
            if (fileName.Contains(
                    "Very Rare",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Very Rare";
            }
            //same with uncommon before common
            if (fileName.Contains(
                    "Uncommon",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Uncommon";
            }

            if (fileName.Contains(
                    "Common",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Common";
            }


            if (fileName.Contains(
                    "Legendary",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Legendary";
            }

            if (fileName.Contains(
                    "Artifact",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Artifact";
            }

            if (fileName.Contains(
                    "Rare",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Rare";
            }

            if (fileName.Contains(
                    "Non Magic",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Non Magic";
            }

            if (fileName.Contains(
                    "Materials",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Materials";
            }

            if (fileName.Contains(
                    "Dragonmarks",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Dragonmarks";
            }
            //added master as it would return at the end an unknown for it.
            if(fileName.Contains(
                    "Master",
                    StringComparison.OrdinalIgnoreCase))
            {
                return "Master";
            }

            return "Unknown";
        }
    }
}