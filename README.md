# Financnidlo

Financnidlo is a transaction simplification tool for group finances. You can give it a list of transactions, it will print out
other transactions representing the same debts, but they will be reduced to 1-1 transactions and there will be less of them.
Output format is the same as input format, so that you can run incrementally simplify debts whenever you need.

## Input / output format

Input is newline sensitive, words (tokens) can be separated by any number of whitespace characters (no newlines).

* comments
    * `# line starting with # is a comment`
    * will be omitted in the output
* definitions
    * `def person name [aliases]`
        * any number of aliases is allowed
        * names are case insensitive
    * `def group name members`
        * group can have other groups as a member
        * group is a set of people
    * `def currency name`
* transactions
    * `somebody paid 123.4currency for someone`
        * `somebody` and `someone` can be a list of people, also groups and aliases
        * transactions between groups of people will be divided equally between all members

### Example

For input:
```
def person John j jb
def person George g gr
def person Mary
def group all jb g Mary
def currency usd

John paid 120usd for all
all paid 30usd for g
Mary paid 100usd for John
```

This will be the output:
```
def person John j jb
def person George g gr
def person Mary
def group all jb g Mary
def currency usd

George paid 60usd for Mary
John paid 10usd for Mary
```

Transaction in the input and in the output result to equivalent debts between people. It's just not obvious that they do.

## Build

To build, clone the repo and run `make build`. Might work only with the latest GCC.

The project can also be build by CMake, but it's only a secondary option to support CLion, which is not happy with simple Makefile. 

## Usage

```sh
# print it out
./financnidlo < transactions

# save the result
./financnidlo < transactions > transactions.new
```

## Implementation

Whole project was implemented using custom functional iterators. For more details, see [ITERS.md](./ITERS.md)

## Performance

My laptop is able to crunch worst possible 100MB input in less than 10s. Worst possible means all definitions, no transactions. Because definitions force reallocation. Input of the same size with mainly transaction takes about 3s. For my needs, that's fast enough.
