namespace DndItemGenerator.Services
{
    public class DicePlan
    {
        private static readonly int[] StandardDice = //dice commonly used in DND
        {
            4, 6, 8, 10, 12, 20, 100
        };

        public List<int> Sides { get; }//which dice to use so if its 20 it uses a D20 or 20,20 means 2d20

        public int Capacity //tells how many unique combos of dice can be used
        {
            get
            {
                int total = 1;

                foreach (int sides in Sides)// is you have 1d20 the capacity is 20, if you have 2d20s you have 400 combos which is 20 * 20. or if say you have a d4 and a d6 the capacity is 24 4*6
                {
                    total *= sides;
                }

                return total;
            }
        }

        public string Label // groups the sides together and returns the dice name. so if you have 20, 20 it will return 2d20s.
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

        public static DicePlan Create(int itemCount)// decide which dice the user needs
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
                        .ToList(); // keep only the combo that will actually work.

                if (possible.Count > 0)
                {
                    return new DicePlan(possible[0]);
                }
            }

            throw new InvalidOperationException(
                "Too many items for the current dice system.");
        }

        private static int Product(IEnumerable<int> values) //multiply the dice together
        {
            int result = 1;

            foreach (int value in values)
            {
                result *= value;
            }

            return result;
        }

        private static IEnumerable<int[]>
            GetCombinations( //recursive method to create possible combos of dice picking the best one
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

        public List<int> RollDigital()//if the user doesnt want to enter a physical dice roll they can use this instead
        {
            List<int> results = new();

            foreach (int sides in Sides)
            {
                results.Add(
                    Random.Shared.Next(1, sides + 1));
            }// picks a random number from the die. so if you have a d6 and a d20 it would pick a random item 1-6 and then again for 1-20. so it could return 5 and 17.

            return results;
        }

        public int ToTableNumber(//ataches the roll to a table entry number
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
            string[] pieces = text.Split(//takes the physical dice roll entered by the user.
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