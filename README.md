# JavaStringHash
Find Java strings that hash to the same values

## Overview
The Java programming language permits the use of strings
within switch / case statements.  This is great ...
except that the code coverage tools cannot recognize
this and will report incomplete coverage:

```java
public class test {

	public static  void runSwitch( String s) {

		// Run the switch
		switch( s) {

		case "A":
			System.out.println( "Got an A");
			break;

		case "B":
			System.out.println( "Got a B");
			break;

		default:
			System.out.println( "Using the default");

		}

	}

	public static void main( String [] args) {

		runSwitch( "A");
		runSwitch( "B");
		runSwitch( "C");

		// Code coverage will show only 3 of 5 paths completed

	}

}
```

This is because, underneath, the Java compiler generates the following:

> Compute the hash of the string
> Jump through a lookup table
> Check that the strings are identical
> If not identical, jump to the default (if any)
> If identical, process

To obtain a 100% code coverage, strings with identical hashes need to be
passed through the switch as well.  If completely arbitrary strings can
be passed through to the switch, this task is very easy.  But usually
the strings are confined to alphanumeric (or even less).

## The `smallAllHash` program
It is simple enough to write a program that begins constructing strings,
computing the hash, and reporting on equality.  But such programs can be
very inefficient - especially if they do not track the hash values already
seen and thus generate repitive checks.

On the other hand, recording every string encountered and built would
simply swamp memory.

But there is a third way.

The `smallAllHash` program allocates slightly more than 512 MB (the
4 billion hash space recorded by 1 bit per hash) to accomplish its
task.  The program uses a vector to track its current position and
the bit array to track which hashes have already been encountered.

The strings identified are not necessarily the smallest strings
that match to the input string hash.  The speed of finding a match,
however, far outweigh the size restriction.

## Back to the example
Add the following lines:

```java
	// Using 12 as the maximum length
	runSwitch( "zZZZN9Or9cY");	// same hash as "A"
	runSwitch( "zZZZN9Or9cZ");	// same hash as "B"

	// Using 8 as the maximum length
	runSwitch( "qBJFB9KH");	// same hash as "A"
	runSwitch( "qBJFB9KI");	// same hash as "B"
```

And now the code coverage will be 100%!

## Future possibilities
* Changing the wait container to an array would allow the smaller
strings to be processed first.  It should be noted, however, that
changing the container to an STL list **DOUBLES** the run time, as
there is a cost to the space allocations.
* Even using upper, lower, and digit characters, strings of length
4 requires fewer than 15 million calculations.  These could be done
upfront before starting longer strings, which would cut many longer
strings from consideration.
* Multi-threading can be employed ... but the cost of synchronization
might overwhelm the additional processing.  Of course, each thread
could maintain its own 512 MB list (and ruin processor cache), or
each thread might process strings within a particular range of
hashes, or one thread could be dedicated to the update while still
allowing read access to the other threads.

