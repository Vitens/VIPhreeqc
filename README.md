![Vitens](https://github.com/AbelHeinsbroek/VIPhreeqc/raw/master/vitens.png)
# VIPhreeqc
Extension of the IPhreeqc 3.3.7 module ([Parkhurst&Appello](http://wwwbrr.cr.usgs.gov/projects/GWC_coupled/phreeqc/)).
This extension aims to add more flexibility to the IPhreeqc module by exposing more information to the C extension. Instead of relying on SELECTED_OUTPUT this extensions enables users to directly gather information such as the pH, SC, speciation, element totals etc. of solutions.

## Implemented Functionality
The following functions are implemented in addition to the standard IPhreeqc functionality:
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
### Error values
The following values are returned on error
```C
-99  = Invalid IPhreeqc Instance
-999 = Solution Not Found
```

## About Vitens
Vitens is the largest drinking water company in The Netherlands. We deliver top quality drinking water to 5.6 million people and companies in the provinces Flevoland, Fryslân, Gelderland, Utrecht and Overijssel and some municipalities in Drenthe and Noord-Holland. Annually we deliver 350 million m³ water with 1,400 employees, 100 water treatment works and 49,000 kilometres of water mains.

One of our main focus points is using advanced water quality, quantity and hydraulics models to further improve and optimize our treatment and distribution processes.
