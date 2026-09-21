using System;
using System.IO;
using System.Windows;
using DndItemGenerator.Models;
using DndItemGenerator.Services;

namespace DndItemGenerator
{
    public partial class MainWindow : Window
    {
        private List<MagicItem> _allItems = new();
        private List<RollTableEntry> _currentTable = new();
        private DicePlan? _dicePlan;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(
            object sender,
            RoutedEventArgs e)
        {
            try
            {
                string dataFolder =
                    Path.Combine(
                        AppContext.BaseDirectory,
                        "Data");

                _allItems =
                    CsvItemLoader.LoadFolder(dataFolder);

                if (_allItems.Count == 0)
                {
                    MessageBox.Show(
                        $"No CSV items were found.\n\n" +
                        $"The program looked here:\n{dataFolder}");

                    return;
                }

                PopulateFilters();

                ItemCountText.Text =
                    $"Loaded {_allItems.Count} total items.";
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    $"Startup Error:\n\n" +
                    $"{ex.Message}\n\n" +
                    $"{ex.StackTrace}",
                    "Startup Error",
                    MessageBoxButton.OK,
                    MessageBoxImage.Error);
            }
        }

        private void PopulateFilters()
        {
            RarityComboBox.Items.Clear();
            TypeComboBox.Items.Clear();
            SourceComboBox.Items.Clear();

            RarityComboBox.Items.Add("Any");
            TypeComboBox.Items.Add("Any");
            SourceComboBox.Items.Add("Any");

            IEnumerable<string> rarities =
                _allItems
                    .Select(item => item.Rarity)
                    .Where(rarity =>
                        !string.IsNullOrWhiteSpace(rarity))
                    .Distinct(
                        StringComparer.OrdinalIgnoreCase)
                    .OrderBy(rarity => rarity);

            foreach (string rarity in rarities)
            {
                RarityComboBox.Items.Add(rarity);
            }

            IEnumerable<string> types =
                _allItems
                    .Select(item => item.Type)
                    .Where(type =>
                        !string.IsNullOrWhiteSpace(type))
                    .Distinct(
                        StringComparer.OrdinalIgnoreCase)
                    .OrderBy(type => type);

            foreach (string type in types)
            {
                TypeComboBox.Items.Add(type);
            }

            IEnumerable<string> sources =
                _allItems
                    .Select(item => item.Source)
                    .Where(source =>
                        !string.IsNullOrWhiteSpace(source))
                    .Distinct(
                        StringComparer.OrdinalIgnoreCase)
                    .OrderBy(source => source);

            foreach (string source in sources)
            {
                SourceComboBox.Items.Add(source);
            }

            RarityComboBox.SelectedIndex = 0;
            TypeComboBox.SelectedIndex = 0;
            SourceComboBox.SelectedIndex = 0;
        }

        private void GenerateTable_Click(
            object sender,
            RoutedEventArgs e)
        {
            GenerateTable(false);
        }

        private void GenerateTable(bool randomize)
        {
            string selectedRarity =
                RarityComboBox.SelectedItem?.ToString()
                ?? "Any";

            string selectedType =
                TypeComboBox.SelectedItem?.ToString()
                ?? "Any";

            string selectedSource =
                SourceComboBox.SelectedItem?.ToString()
                ?? "Any";

            List<MagicItem> filtered =
                ItemFilter.Filter(
                    _allItems,
                    selectedRarity,
                    selectedType,
                    selectedSource);

            if (filtered.Count == 0)
            {
                MessageBox.Show(
                    "No items matched those filters.");

                return;
            }

            _currentTable =
                RollTableBuilder.Build(
                    filtered,
                    randomize);

            _dicePlan =
                DicePlan.Create(
                    _currentTable.Count);

            RefreshTableDisplay();

            ItemCountText.Text =
                $"Table contains {_currentTable.Count} items.";

            RequiredDiceText.Text =
                $"Required Dice: {_dicePlan.Label}";

            if (_dicePlan.Sides.Count == 1)
            {
                RollHelpText.Text =
                    $"Enter one number from 1-" +
                    $"{_dicePlan.Sides[0]}.";
            }
            else
            {
                RollHelpText.Text =
                    "Enter each die separately in order. " +
                    "Do not add them together. Example: 7,14";
            }

            ClearResult();
        }

        private void RandomizeTable_Click(
            object sender,
            RoutedEventArgs e)
        {
            if (_currentTable.Count == 0)
            {
                GenerateTable(true);
                return;
            }

            List<MagicItem> currentItems =
                _currentTable
                    .Select(entry => entry.Item)
                    .ToList();

            _currentTable =
                RollTableBuilder.Build(
                    currentItems,
                    true);

            RefreshTableDisplay();

            ClearResult();
        }

        private void RefreshTableDisplay()
        {
            TableGrid.ItemsSource = null;
            TableGrid.ItemsSource = _currentTable;
        }

        private void PhysicalRoll_Click(
            object sender,
            RoutedEventArgs e)
        {
            if (_dicePlan == null ||
                _currentTable.Count == 0)
            {
                MessageBox.Show(
                    "Generate a table first.");

                return;
            }

            try
            {
                List<int> rolls =
                    DicePlan.ParseRolls(
                        PhysicalRollTextBox.Text);

                ResolveRoll(
                    rolls,
                    "Physical");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void DigitalRoll_Click(
            object sender,
            RoutedEventArgs e)
        {
            //if you dont generate a table the digital dice roller wont have what dice it needs
            if (_dicePlan == null ||
                _currentTable.Count == 0)
            {
                MessageBox.Show(
                    "Generate a table first.");

                return;
            }

            List<int> rolls =
                _dicePlan.RollDigital();

            DigitalRollText.Text =
                $"Rolled: {string.Join(", ", rolls)}";

            ResolveRoll(
                rolls,
                "Digital");
        }

        private void ResolveRoll(
            IReadOnlyList<int> rolls,
            string rollType)
        {
            if (_dicePlan == null)
                return;
            //first trying the table in case of a higher number rolled then whats on the table.
            try
            {
                int tableNumber =
                    _dicePlan.ToTableNumber(rolls);

                if (tableNumber >
                    _currentTable.Count)
                {
                    ResultNameText.Text =
                        "Reroll";

                    ResultDetailsText.Text =
                        $"{rollType} roll: " +
                        $"{string.Join(", ", rolls)}\n" +
                        $"Generated table number: " +
                        $"{tableNumber}\n\n" +
                        "That number is outside the " +
                        "current table. Roll again.";

                    DescriptionTextBox.Text = "";

                    return;
                }

                RollTableEntry result =
                    _currentTable[
                        tableNumber - 1];

                ShowResult(
                    result,
                    rolls,
                    rollType);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void ShowResult(
            RollTableEntry entry,
            IReadOnlyList<int> rolls,
            string rollType)
        {
            MagicItem item = entry.Item;

            ResultNameText.Text =
                item.Name;

            ResultDetailsText.Text =
                $"{rollType} Roll: " +
                $"{string.Join(", ", rolls)}\n" +
                $"Table Number: {entry.RollNumber}\n\n" +
                $"Rarity: {item.Rarity}\n" +
                $"Type: {item.Type}\n" +
                $"Source: {item.Source}\n" +
                $"Attunement: {item.Attunement}\n" +
                $"Price: {item.Price}\n" +
                $"Damage: {item.Damage}";

            DescriptionTextBox.Text =
                item.Description;
        }

        private void ClearResult()
        {
            ResultNameText.Text = "";
            ResultDetailsText.Text = "";
            DescriptionTextBox.Text = "";
            DigitalRollText.Text = "";
            PhysicalRollTextBox.Text = "";
        }
    }
}