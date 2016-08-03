![Vitens](https://github.com/AbelHeinsbroek/VIPhreeqc/raw/master/vitens.png)

[![Build Status](https://travis-ci.org/AbelHeinsbroek/VIPhreeqc.svg?branch=master)](https://travis-ci.org/AbelHeinsbroek/VIPhreeqc) ![Build Status](https://ci.appveyor.com/api/projects/status/github/abelheinsbroek/viphreeqc?svg=true)

# VIPhreeqc
Extension of the IPhreeqc 3.3.7 module ([Parkhurst&Appello](http://wwwbrr.cr.usgs.gov/projects/GWC_coupled/phreeqc/)).
This extension aims to add more flexibility to the IPhreeqc module by exposing more information to the C extension. Instead of relying on SELECTED_OUTPUT this extensions enables users to directly gather information such as the pH, SC, speciation, element totals etc. of solutions.

## Implemented Functionality
The following functions are implemented in addition to the standard IPhreeqc functionality:
```C
  /**
   * Returns the pH of the specified solution
   */
  double                  GetPH(int solution);
  /**
   * Returns the pe of the specified solution
   */
  double                  GetPe(int solution);
  /**
   * Returns the specific conductance (in uS/cm) of the specified solution
   */
  double                  GetSC(int solution);
  /**
   * Returns the amount (in mol) of an element (e.g. C(-4), Ca, etc.)
   */
  double                  GetTotal(int solution, const char *string);
  /**
   * Returns the total amount (in mol) of an element (e.g. C, Ca, etc.)
   */
  double                  GetTotalElement(int solution, const char *string);
  /**
   * Returns the amount of moles (in mol) of a species (e.g. Ca+2, OH-, etc.)
   */
  double                  GetMoles(int solution, const char *species);
  /**
   * Returns the molality (in mol/kgW) of a species (e.g. Ca+2, OH-, etc.)
   */
  double                  GetMolality(int solution, const char *species);
  /**
   * Returns a comma separated list of all the species in the selected solution
   */
  const char*             GetSpecies(int solution);
  /**
   * Returns the Solubility Index of a phase in a solution
   */
  double                  GetSI(int solution, const char *phase);
  /**
   * Returns a comma separated list of all the phases in the selected solution
   */
  const char*             GetPhases(int solution);
  /**
   * Returns a comma separated list of all the elements in the selected solution
   */
  const char*             GetElements(int solution);
```
### Error values
The following values are returned on error
```C
-99  = Invalid IPhreeqc Instance
-999 = Solution Not Found
```

## About Vitens
Vitens is the largest drinking water company in The Netherlands. We deliver top quality drinking water to 5.6 million people and companies in the provinces Flevoland, Fryslân, Gelderland, Utrecht and Overijssel and some municipalities in Drenthe and Noord-Holland. Annually we deliver 350 million m³ water with 1,400 employees, 100 water treatment works and 49,000 kilometres of water mains.

One of our main focus points is using advanced water quality, quantity and hydraulics models to further improve and optimize our treatment and distribution processes.

## Licence

Copyright 2016 Vitens

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
