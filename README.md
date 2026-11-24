# Лабораторна робота №1 — Базові ідеї ООП (C++17)

## Обраний варіант

| | |
|---|---|
| **Перелік 1** | № 6 — дерева з довільною кількістю дітей, бінарні дерева, дерева бінарного пошуку (`**`) |
| **Додаткове завдання з переліку 1** | **a** (`+*`): різні варіанти обходу дерев, пошук за шляхом у дереві |
| **Перелік 2** | «Текстові документи» (структура документа — природне дерево) |
| **Рівень складності (перелік 2)** | **b** (включає рівень a — класи даних; рівень c не реалізовано) |
| **Реалізовані зірочки (перелік 1)** | `**` (базове завдання) + `*` (додаткове a) = **3** |

Не реалізовано свідомо: додаткове завдання б (механізми видалення піддерев), рівень c (видобування з реальних документів), запис/читання з файлу, графічний інтерфейс, інтернаціоналізація.

Стиль коду: Google C++ Style Guide (відступ 2 пробіли, поля класів із суфіксом `_`, `CamelCase` для типів).

## Збірка та запуск

```bash
make            # збирає build/lab1 та build/lab1_tests
make test       # запускає юніт-тести
./build/lab1 demo --seed 42      # демонстрація всіх дерев на всіх типах даних
./build/lab1 docdemo             # документ + правила оформлення + звіт
./build/lab1 shell               # інтерактивний командний рядок (за замовчуванням)
```

Або через CMake: `cmake -S . -B build && cmake --build build && ctest --test-dir build`.
Ключ `--verbose` вмикає логування, `--seed N` задає seed генератора випадкових даних.
Документація (Doxygen): `doxygen docs/Doxyfile`.

## Команди командного рядка

```
new <kind> <type> [branching]   kind: general | general-std | binary | bst
                                type: int | double | string | vector | element
insert <v>                      додати туди, куди вирішить саме дерево
add <parent> <v>                додати як дитину вузла <parent>
remove <v>                      видалити перший вузол зі значенням <v>
removechild <parent> <i>        видалити i-ту дитину (з піддеревом)
contains <v> | path <v>         пошук за значенням / шлях від кореня
haspath <v1> <v2> ...           чи є послідовність значень шляхом від кореня
traverse <pre|post|level|in>    обхід
random <count> [seed]           заповнити випадковими даними
print | info | clear | demo | docdemo | help | quit
```

Формат значень: `5`, `2.5`, `word`, вектор `1,2,3`, елемент документа
`title:Назва:Автор`, `section:1:Заголовок`, `paragraph:Текст_абзацу.`, `list:a;b`,
`table:Підпис:2:3`, `image:file.png:Підпис`, `formula:1:E=mc2`, `source:1:Книга` (`_` = пробіл).

Приклад:
```
new general string 2
insert root
add root a
add a c
path c            # "root" -> "a" -> "c"
haspath root a c  # yes
traverse level
```

## Архітектура

```
include/lab1/common    logger, TextTraits (текстове подання), RandomSource/RandomValue
include/lab1/tree      ITree, TreeBase, GeneralTree, BinaryTree, BinarySearchTree,
                       обходи (strategy), сховища дітей, фабрика та fillRandom
include/lab1/document  елементи документа, Document, правила та валідатор
include/lab1/cli       сесія над деревом довільного типу, shell, демонстрації
src/, tests/           реалізації нешаблонного коду, юніт-тести (213 перевірок)
```

* `ITree<T>` → `TreeBase<T>` (усі алгоритми через `INodeView`: обходи, пошук шляху, `toString`) → `GeneralTree<T, Storage>` та `BinaryTree<T>` → `BinarySearchTree<T, Compare>`.
* Дерево з довільною кількістю дітей має два сховища дітей на вибір (шаблонний параметр): **власний зв'язний список** (`LinkedChildList`) та **бібліотечний `std::list`** (`StdChildList`).
* Обходи: pre-order, post-order, level-order (BFS), in-order (для бінарних — класичний; для довільних — «перша дитина, вузол, решта»). Кожен обхід — окрема стратегія `ITraversal<T>`, яка працює з будь-яким деревом.
* Пошук за шляхом: `pathTo(value)` (шлях значень від кореня до вузла), `hasPath(path)` (чи існує такий шлях вниз від кореня). У BST `pathTo` використовує впорядкованість (O(h)).
* Видалення: за значенням (`removeValue`) та за батьківським вузлом і індексом (`removeChild`). У загальному та бінарному деревах видаляється вузол разом із піддеревом; у BST `removeValue` — класичне видалення одного елемента.
* Дерева працюють з `int`, `double`, `std::string`, `std::vector<int>` та власним класом `ElementHandle` (елемент документа) — див. `./build/lab1 demo`.
* Документ — це `GeneralTree<ElementHandle>`: корінь `DocumentTitle`, далі розділи, абзаци, списки, таблиці, зображення, формули, джерела.
* Рівень b: 6 правил оформлення (`HeadingLevelRule`, `EmptySectionRule`, `CaptionRule`, `ParagraphStyleRule`, `CitationRule`, `FormulaNumberingRule`), валідатор і звіт про знайдені помилки (`ValidationReport`).

## Звіт про реалізовані елементи

### Класи та типи — **58** (мінімум 9)
* **42 класи/шаблони класів:** `ITree`, `TreeBase`, `GeneralTree`, `GeneralNode`, `BinaryTree`, `BinaryNode`, `BinarySearchTree`, `INodeView`, `ITraversal`, `PreOrderTraversal`, `PostOrderTraversal`, `LevelOrderTraversal`, `InOrderTraversal`, `LinkedChildList`, `StdChildList`, `RandomSource`, `Logger`, `ITreeSession`, `TreeSession`, `Shell`, `DocumentElement`, `DocumentTitle`, `Section`, `Paragraph`, `ListBlock`, `TableBlock`, `ImageBlock`, `FormulaBlock`, `SourceEntry`, `ElementHandle`, `Document`, `Violation`, `ValidationReport`, `IFormattingRule`, `ElementRule`, `HeadingLevelRule`, `EmptySectionRule`, `CaptionRule`, `ParagraphStyleRule`, `CitationRule`, `FormulaNumberingRule`, `DocumentValidator`.
* **16 структур:** `VisitContext`, `Link` (вузол власного списку), а також trait-структури статичного поліморфізму: `TextTraits` (4: загальна + арифметичні/`string`/`vector`), `RandomValue` (5), `ValueParser` (5).
* Також 4 `enum class`: `TraversalOrder`, `LogLevel`, `ElementKind`, `Severity`.

### Поля — **51** (мінімум 15)
`TreeSession` (3), `Shell` (4), `Logger` (2), `RandomSource` (1), `Document` (1), `DocumentTitle` (2), `Section` (2), `Paragraph` (1), `ListBlock` (2), `TableBlock` (3), `ImageBlock` (2), `FormulaBlock` (2), `SourceEntry` (2), `ElementHandle` (1), `Violation` (4), `ValidationReport` (1), `DocumentValidator` (1), `BinarySearchTree` (1), `GeneralNode` (2), `BinaryNode` (3), `BinaryTree` (1), `LinkedChildList` (3), `Link` (2), `StdChildList` (1), `GeneralTree` (2), `VisitContext` (2).

### Нетривіальні методи та функції — **81** (мінімум 25)
* **Дерева (28):** `TreeBase::{size, height, pathTo, hasPath, toVector, toString, header, findPath, matchPath}`; `GeneralNode::childSlots`; `GeneralTree::{insert, addChild, removeValue, removeChild, findFirstWithRoom, findByValue, removeFrom}`; `BinaryTree::{insert, addChild, removeValue, removeChild, toString, findByValue, removeFrom, print}`; `BinarySearchTree::{insert, removeValue, pathTo}`.
* **Обходи та сховища (12):** `PreOrderTraversal::visit`, `PostOrderTraversal::visit`, `LevelOrderTraversal::traverse`, `InOrderTraversal::visit`, `makeTraversal`, `parseOrder`, `LinkedChildList::{append, at, removeAt, ~LinkedChildList}`, `StdChildList::{at, removeAt}`.
* **Генерація та текст (8):** `fillRandom`, `makeTree`, `RandomSource::{word, capitalizedWord}`, `RandomValue<vector>::generate`, `RandomValue<ElementHandle>::generate`, `TextTraits<vector>::toText`, `joinValues`.
* **Документ і правила (20):** `Paragraph::{sentences, citations}`, `parseElement`, `ElementHandle::operator=`, `Document::{add, tableOfContents}`, `makeSampleDocument`, `ElementRule::check`, `HeadingLevelRule::checkElement`, `EmptySectionRule::checkElement`, `CaptionRule::checkElement`, `ParagraphStyleRule::checkElement`, `CitationRule::check`, `FormulaNumberingRule::check`, `DocumentValidator::{validate, withStandardRules}`, `ValidationReport::{count, countForRule, toString}`, `Violation::toString`.
* **Командний рядок (13):** `TreeSession::execute`, `ValueParser<int/double/vector<int>>::parse`, `parseIndex`, `makeSession`, `tokenize`, `Shell::{run, handle}`, `runStructuresDemo`, `runDocumentDemo`, `exerciseTree`, `demonstrate`.

Тривіальні getters/setters/конструктори в підрахунок не входять (вони є у `DocumentTitle`, `Section`, `Violation` тощо).

### Ієрархії успадкування — **6** (мінімум 2; одна з ≥3 класів)
1. `ITree` → `TreeBase` → {`GeneralTree`, `BinaryTree` → `BinarySearchTree`} — 5 класів, 4 рівні.
2. `DocumentElement` → {`DocumentTitle`, `Section`, `Paragraph`, `ListBlock`, `TableBlock`, `ImageBlock`, `FormulaBlock`, `SourceEntry`} — 9 класів.
3. `IFormattingRule` → {`ElementRule` → {`HeadingLevelRule`, `EmptySectionRule`, `CaptionRule`, `ParagraphStyleRule`}, `CitationRule`, `FormulaNumberingRule`} — 8 класів, 3 рівні.
4. `ITraversal` → 4 стратегії обходу.
5. `INodeView` → {`GeneralNode`, `BinaryNode`}.
6. `ITreeSession` → `TreeSession<T>`.

### Незалежні випадки поліморфізму — **10** (мінімум 3; є статичний і динамічний)
**Динамічний (6)** — клієнтський код працює лише через інтерфейс:
1. `ITree`: `fillRandom`, `exerciseTree`, `TreeSession` працюють з `ITree<T>&` і не знають конкретного дерева (перевизначення `insert`, `addChild`, `removeValue`, `pathTo`, `toString`, …).
2. `ITraversal`: `TreeBase::traverse` обирає стратегію обходу під час виконання.
3. `IFormattingRule`: `DocumentValidator` проганяє правила через інтерфейс.
4. `DocumentElement`: `Document::add` (віртуальний `canContain`), `ElementHandle` (`describe`, `key`, `clone`).
5. `ITreeSession`: `Shell` працює з деревом, не знаючи типу значень.
6. `INodeView`: одна реалізація кожного обходу обслуговує всі види вузлів.

**Статичний (4):**
7. Шаблони дерев за типом значення: `GeneralTree<T, …>`, `BinaryTree<T>`, `BinarySearchTree<T, Compare>`.
8. Policy-параметр `GeneralTree<T, LinkedChildList | StdChildList>` (template template parameter).
9. Trait-спеціалізації `TextTraits<T>`, `RandomValue<T>`, `ValueParser<T>`.
10. Шаблони функцій `fillRandom<T>`, `makeTree<T>`, `makeTraversal<T>`, `joinValues<T>`, `exerciseTree<T>`.

### Інкапсуляція
Усі поля приватні (або `protected` лише там, де їх потрібно успадкувати: корінь `BinaryTree`); допоміжні методи (`findByValue`, `removeFrom`, `header`, …) приватні/захищені; доступ до даних елементів — через getters; `ElementRule::checkElement` — `protected` (Template Method).

## Використання LLM
LLM (Claude, Anthropic) давала ідеї як реалізувати більшість функцій. 
LLM самостійно писала функціонал для BinarySearchTree::removeValue, LinkedChildList::removeAt, Шаблони з template <typename> class ChildStorage, ElementHandle, та написала красивий "інтерфейс" в консоль і виправляла помилки в коді.
Документацію згенеровано за допомогою LLM (Claude, Anthropic)

