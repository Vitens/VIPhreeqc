# VIPhreeqc ![Vitens](https://github.com/AbelHeinsbroek/VIPhreeqc/raw/master/vitens.png)
Vitens extension of the IPhreeqc 3.3.7 module ([Parkhurst&Appello](http://wwwbrr.cr.usgs.gov/projects/GWC_coupled/phreeqc/)).
This extension aims to add more flexibility to the IPhreeqc module by exposing more information to the C extension. Instead of relying on SELECTED_OUTPUT this extensions enables users to directly gather information such as the pH, SC, speciation, element totals etc. of solutions.

## Implemented Functionality
The following functions are implemented:
```C
  double                  GetPH(int solution);
  double                  GetPe(int solution);
  double                  GetSC(int solution);
  double                  GetTotal(int solution, const char *string);
  double                  GetTotalElement(int solution, const char *string);
  double                  GetMoles(int solution, const char *species);
  double                  GetMolality(int solution, const char *species);
  const char*             GetSpecies(int solution);
  double                  GetSI(int solution, const char *phase);
  const char*             GetPhases(int solution);
  const char*             GetElements(int solution);
```

