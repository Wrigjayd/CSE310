namespace DndItemGenerator.Services
{
    public class DicePlan
    {
        private static readonly int[] StandardDice = //dice commonly used in DND
        {
            4, 6, 8, 10, 12, 20, 100
        };

        public List<int> Sides { get; }

        public int Capacity
        {
            get
            {
                int total = 1;

                foreach (int sides in Sides)
                {
                    total *= sides;
                }

                return total;
            }
        }

        public string Label
        {
            get
            {
                return string.Join(
                    " + ",
                    Sides
                        .GroupBy(x => x)
                        .Select(group =>
                            $"{group.Count()}d{group.Key}")
                );
            }
        }

        private DicePlan(IEnumerable<int> sides)
        {
            Sides = sides.ToList();
        }

        public static DicePlan Create(int itemCount)
        {
            if (itemCount <= 0)
                throw new ArgumentException(
                    "There must be at least one item.");

            // First try one die.
            foreach (int die in StandardDice)
            {
                if (die >= itemCount)
                {
                    return new DicePlan(new[] { die });
                }
            }

            // If one die isn't enough, find a
            // combination of standard dice.
            for (int diceCount = 2;
                 diceCount <= 4;
                 diceCount++)
            {
                var possible =
                    GetCombinations(
                            StandardDice,
                            diceCount)
                        .Where(combo =>
                            Product(combo) >= itemCount)
                        .OrderBy(combo =>
                            Product(combo))
                        .ThenBy(combo =>
                            combo.Max())
                        .ToList();

                if (possible.Count > 0)
                {
                    return new DicePlan(possible[0]);
                }
            }

            throw new InvalidOperationException(
                "Too many items for the current dice system.");
        }

        private static int Product(IEnumerable<int> values)
        {
            int result = 1;

            foreach (int value in values)
            {
                result *= value;
            }

            return result;
        }

        private static IEnumerable<int[]>
            GetCombinations(
                int[] values,
                int length,
                int startIndex = 0)
        {
            if (length == 0)
            {
                yield return Array.Empty<int>();
                yield break;
            }

            for (int i = startIndex;
                 i < values.Length;
                 i++)
            {
                foreach (int[] tail in GetCombinations(
                             values,
                             length - 1,
                             i))
                {
                    int[] result = new int[tail.Length + 1];

                    result[0] = values[i];

                    Array.Copy(
                        tail,
                        0,
                        result,
                        1,
                        tail.Length);

                    yield return result;
                }
            }
        }

        public List<int> RollDigital()
        {
            List<int> results = new();

            foreach (int sides in Sides)
            {
                results.Add(
                    Random.Shared.Next(1, sides + 1));
            }

            return results;
        }

        public int ToTableNumber(
            IReadOnlyList<int> rolls)
        {
            if (rolls.Count != Sides.Count)
            {
                throw new ArgumentException(
                    $"Expected {Sides.Count} dice.");
            }

            int number = 0;

            for (int i = 0; i < rolls.Count; i++)
            {
                int roll = rolls[i];
                int sides = Sides[i];

                if (roll < 1 || roll > sides)
                {
                    throw new ArgumentException(
                        $"Roll {i + 1} must be between " +
                        $"1 and {sides}.");
                }

                number =
                    number * sides +
                    (roll - 1);
            }

            return number + 1;
        }

        public static List<int> ParseRolls(string text)
        {
            string[] pieces = text.Split(
                new[]
                {
                    ',',
                    ' ',
                    ';',
                    '/'
                },
                StringSplitOptions.RemoveEmptyEntries);

            List<int> rolls = new();

            foreach (string piece in pieces)
            {
                if (!int.TryParse(piece, out int roll))
                {
                    throw new ArgumentException(
                        $"'{piece}' is not a valid roll.");
                }

                rolls.Add(roll);
            }

            return rolls;
        }
    }
}