# Tree generation via L System

## Table of content

- [What is L-System?](#what-is-l-system)
- [utymap's implementation](#utymaps-implementation)

OpenStreetMap data provides quite a lot of additional information which might be exposed using 3D procedural generation engines like utymap. For tree generation, there are several useful attributes available [1]:
* _leaf_type_ describes the type of leaves, for instance broadleaved or needleleave
* _leaf_cycle_ describes the phenology of leaves, for instance evergreen or deciduouc
* _height_ defines height of the tree
* _diameter_crown_ defines for the diameter of the crown of foliage of the tree
* _genus, species, taxon_ – scientific  names

Unfortunately, not all of attributes are widely used. So, current solution based on L-System uses only leaf_type, but it can be easily extended to consume all of them.

## What is L-System?

By wikipedia definition: "_an **L-system** or **Lindenmayer system** is a parallel **rewriting system** and a type of **formal grammar**. An L-system consists of an alphabet of symbols that can be used to make strings, a collection of **production rules** that expand each symbol into some larger string of symbols, an initial "**axiom**" string from which to begin construction, and a mechanism for translating the generated strings into geometric structures.. The **recursive** nature of the L-system rules leads to **self-similarity** and thereby, **fractal-like** forms are easy to describe with an L-system. Plant models and natural-looking organic forms are easy to define, as by increasing the recursion level the form slowly 'grows' and becomes more complex_"[2].

Let's look at example of the simplest class of L-systems [3]. Imagine string built of two letters _a_ and _b_, which may occur many times in a string. Each letter is associated with a rewriting rule:
```
a -> ab
b -> a
```
 The rule `a -> ab` means that the letter a is replaced by the string ab and the rule `b -> a` means that b is to be replaced by a. The rewriting process starts from a initial string called the axiom. Assume that it consists of a single letter b. In the first step the axiom `b` is replaced by a using production `b -> a`. In the second step a is replaced by ab using production `a -> ab`. The word `ab `consists of two letters, both of which are simultaneously replaced in the next derivation step:

```
axiom: b
1. ab
2. aba
3. abaab
4. abaababa
5. abaababaabaab
...
```

Final result above is just a string, so how it can be useful for generating plants? The key idea is how the string is interpreted: each symbol corresponds to one specific turtle command, e.g. move, turn, or even jump. With command execution turtle can "draw" something on canvas or just change its position.

Consider a 2D example. At the beginning, turtle has some state which is defined by position (x, y) and angle, called heading, which is interpreted as the direction in which the turtle is facing. Given step size d and the angle increment δ, turtle can respond to commands represented by the following symbols:

* **F** moves forward a step of length d and draw a line
* **f** moves forward a step of length d without drawing a line
* **+** turns left by angle δ
* **−** turns right by angle δ

Image below shows an example how simple string can be interpreted:

![turtle2d](https://cloud.githubusercontent.com/assets/1611077/21948903/54a28078-d9ee-11e6-9121-770accb4b10b.png)

With these simple rules, we can produce even more complex output:

![self-repeated](https://cloud.githubusercontent.com/assets/1611077/21948990/f8bdda9a-d9ee-11e6-8824-2d383b9acf7b.png)

Here, initial state of the turtle is defined by axiom and production:
* axiom: F − F − F − F
* rule: F -> F − F + F + F F − F − F + F
* n: number of generation.

## utymap's implementation

utymap implements stochastic context-free l-system which means it supports production rule probabilities but it does not care about rules order. So each rule is treated as it is, without checking what rule is before or after.  Let's look at the simplest lsystem file example:
```
generations: 1
angle: 0
scale: 5
axiom: root
root -> ! cylinder @ $ > cone
```

This file models the simplest conifer tree:

![conifer](https://cloud.githubusercontent.com/assets/1611077/21949038/692a8b16-d9ef-11e6-9ff9-b03e5b052f73.png)

File has one generation for rewriting rules, turn angle is set to zero, line width/length scale factor is set to 5 and axiom is root. "root" word has no specific meaning: it is introduced to make grammar more self-describable and on the first rewriting step will be replaced by its production rule with the following string:

`! cylinder @ $ > cone`

**Note**: default width and length is set to 1 meter in mapcss for convenience.

Let's look close at this string and its content. Words with one symbol are parsed as specific rules defined in the following symbol table:
```C++
struct RuleTable : qi::symbols<char, LSystem::RuleType>
{
    RuleTable()
    {
        add
            ("F", forward)
            ("G", jump)
            ("+", turnLeft)
            ("-", turnRight)
            ("|", turnAround)
            ("^", pitchUp)
            ("&", pitchDown)
            ("\\", rollLeft)
            ("/", rollRight)
            ("$", increment)
            ("!", decrement)
            (">", scaleUp)
            ("<", scaleDown)
            ("@", switchStyle)
            ("[", save)
            ("]", restore)
        ;
    }
};
```

Each symbol is mapped to one specific rule which represents turtle command, e.g. move, turn left. Most of them are related to turtle orientation in 3D space, but some are special:

* **$** scales line width up by factor "scale"
* **!** scales line width downby factor "scale"
* **>** scales line length up by factor "scale"
* **<** scales line width down by factor "scale"
* **@** switches to the next style (texture)
* **[** saves turtle state (position, rotation, e.g.) on the stack
* **]** restores turtle state from the stack

Grammar content is parsed using custom implementation and it is written using awesome boost spirit library which provides the way to define grammar in some kind of adopted _Extended Backus-Naur_ form directly in C++ source code [4]. You can find implementation here [5].

As mentioned above, _cylinder_ and _cone_ are special strings known by specific turtle implementation which treats them as cylinder and cone shapes in low poly style (will be discussed later). In general, turtle is implemented as visitor pattern [6] with predefined set of rules which it can handle:

```C++
/// Defines abstract turtle behaviour by utilizing common l-system notation.
class Turtle
{
public:
    virtual void moveForward() {}
    virtual void jumpForward() {}
    virtual void turnLeft() {}
    virtual void turnRight() {}
    virtual void turnAround() {}
    ...
    /// Performs instructions specific for given word.
    virtual void say(const std::string& word) {}
    /// Runs turtle using lsystem provided.
    virtual void run(const LSystem& lsystem);
```
_run_ method is responsible for generating rule list using amount of generations, axiom and production rules. Each rule accepts turtle as visitor:

```C++
for (const auto& rule : rules)
    rule->apply(*this);
```

Having this, we can easily extend _Turtle_ class with different implementations:

```C++
/// Defines turtle which writes its actions using common lsystem grammar notation.
class StringTurtle final : public utymap::lsys::Turtle
{
public:
    void moveForward() override                 { path += "F"; }
    void jumpForward() override                 { path += "G"; }
    ....
    void say(const std::string& word) override  { path += word; }

    std::string path;
};
```

StringTurtle memorizes each rule using its string representation which is quite useful for testing parser:

```C++
BOOST_AUTO_TEST_CASE(GivenGrammarWithProbabilities_WhenParse_ThenStringTurtleBuildsProperPath)
{
    StringTurtle turtle;
    const std::string grammar = "generations: 2\n"
                                "angle: 45\n"
                                "scale: 1.2\n"
                                "axiom: A G\n"
                                "G(0) -> G\n"
                                "G(1) -> F F\n"
                                "F -> G\n"
                                "A -> G G";

    turtle.run(LSystemParser().parse(grammar));

    BOOST_CHECK_EQUAL(turtle.path, "FFFFGG");
}
```

For 3D space, there is _Turtle3d_ class which provides the way to manage turtle state. State is represented by position, heading, line width/height/texture.

For building meshes, utymap uses a turtle implemented by _LSystemGenerator_ class. It is quite simple, knows only three primitives (words) so far:
* **cylinder**  generates cylinder using current width as radius, length as height
* **cone**  generates cone the same way as cylinder
* **sphere**  generates _icosphere_ using width as x,z radius and length as y radius.

So, whenever final string produced by L-system has one of these words then corresponding primitive is generated based on current turtle state and then added to the mesh. Having just such simple implementation, we can generate various trees:

![tree1](https://cloud.githubusercontent.com/assets/1611077/21949323/890b9752-d9f1-11e6-8cf7-27b28c5c4f15.png)
![tree2](https://cloud.githubusercontent.com/assets/1611077/21949330/9619a7b8-d9f1-11e6-93f9-7498d6c450ca.png)

..and not only trees:

![street light](https://cloud.githubusercontent.com/assets/1611077/21949340/acd6d50c-d9f1-11e6-8283-206e0c15271a.png)


1. http://wiki.openstreetmap.org/wiki/Tag:natural%3Dtree
2. https://en.wikipedia.org/wiki/L-system
3. http://algorithmicbotany.org/papers/abop/abop.pdf (The Algorithmic Beauty of Plants is a book by Przemyslaw Prusinkiewicz and Aristid Lindenmayer)
4. http://boost.org/libs/spirit
5. https://github.com/reinterpretcat/utymap/blob/master/core/src/lsys/LSystemParser.cpp
6. https://en.wikipedia.org/wiki/Visitor_pattern
