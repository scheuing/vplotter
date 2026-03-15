# V Plotter project

This project describes my approach to a vertical wall hanging plotting machine. 
My first contact to plotters was in school back in the early 1990s. An HP x-y-plotter, mostly used as an x-t-plotter, was applied in physics lessons. 
I don't really remember the use-case, probably it was just to show us sinusoidal voltage over time. The machine provided the fascination of a hygrometer-writer with cylindrical millimeter paper... but in more electrical. 
In my memory, the paper was suck to the draw-board by some vacuum pump, and although the teachers always advised us to use the caps, these special plotter pens in black and red color were typically dried-out.

Many years later, equipped with some experience of CNC milling machines, a diploma in electrical engineering, and a lot of professional years in embedded system development, I came across some websites with vertical plotters and my fascination to plotting machines rose again. 
Finally investing in a 3D filament plotter, there remained no excuse not to build one. 
But, it would have been too easy to just copy any of the plotter setups that can be found in www, use existing software, get experienced and finally come up with proud on some minor improvements. 
So I started from scratch...

# System setup
Like others, my V Plotter consists of a gondola driven by two stepper motors via strings. A servo on the gondola shall enable a pen lift-off.
Stepper motors and the servo shall be attached via driver to an Arduino. 
The Arduino interprets a list of G codes and coordinates stepper and servo movements.
G code shall use the orthogonal coordinate system of the wall. Thus the transformation to string length resp. stepper movements is a task of the Arduino.

### Gondola
Having an analytic look at e.g. Hektor[^1], separate string mounting positions evoke tilting at the gondola and requires complex compensation[^2]. 
By construction, this can be avoided with a pen concentric mounting using bearings[^3].

### String: cord, chain, belt?
...

### Arduino
The usage of an Arduino was a gut decision: 
I chose an embedded system off-the-shelf as I was too enthusiastic to stay with PCB layout. 
And I decided against the option of a Raspberry Pi, as I didn't want to cope with triggering a shut-down. 
In consequence, the program needs to be somehow provided by another computer (my PC) while plotting - a drawback of not having Pi's operation system, USB or wifi support.

Nevertheless, thoughts, mechanics, algorithms, and hopefully some software should be portable to other embedded platforms.

Evaluating other V plotter projects[^4], two main categories of drawing applications are common: 
the plotting of a vector graphic and the (grey-shaded) pixel drawing. 
My primary interest is vector graphics or even text plotting.
I guess that requires preciseness... 

# Overview
Building-up a V Plotter is a phantastic multi-discipline educational project, covering workmanship skills, engineering, CAD and programming, even art.

And each discipline offers levels of precision and pedantism...
So let's start with some theoretical analysis of [math](docs/math.md) and [kinematics](docs/kinematics.md).


---
---

[^1]: "The daddy of all hanging drawing machines": https://juerglehni.com/works/hektor

[^2]: see e.g. https://math.stackexchange.com/questions/1914608/complex-system-of-equations-to-define-geometry-for-hanging-plotter or https://robotics.stackexchange.com/questions/10607/forward-and-revers-kinematics-for-modified-hanging-plotter

[^3]: see e.g. https://hackaday.io/project/15580-v-plotter/details or https://www.instructables.com/Polargraph-Drawing-Machine/

[^4]: an incomplete, unsorted list of links to other projects: https://github.com/rottaca/VPlotter, https://github.com/euphy/polargraph/wiki, https://github.com/bertlr/vplotter, https://github.com/MarginallyClever/Makelangelo-software/wiki, https://www.homofaciens.de/technics-machines-v-plotter_en.htm, https://www.makerblog.at/2014/09/vertikalplotter-im-selbstbau-teil-1-polargraph-kritzler-makelangelo-und-co/, https://jimlaurwilliams.org/wordpress/?p=6053, https://lizmelchor.com/wall-robot/?v=7d0db380a5b9

