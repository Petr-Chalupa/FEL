#import "@preview/plotst:0.2.0": *
#set par(leading: 0.55em, spacing: 0.55em, first-line-indent: 1.8em, justify: true)
#set text(font: "New Computer Modern")
#show heading: set block(above: 2.5em, below: 1em)
#set page(margin: (x: 1.25in, y: 1.5in), footer: context {
  if counter(page).get().first() == 0 [#align(right, text(14pt)[RPH, 2024/2025])]
  else { align(center, counter(page).display("1")) }
})

#counter(page).update(0)
#align(center)[
  #text(30pt)[== Spam filter - report]
  #v(5em)
  #text(14pt)[== Chalupa Petr, Štácha Martin]
]

#pagebreak()
#set page(numbering: "1", number-align: center)
#set text(14pt)

== Úvod
Tento projekt je týmová úloha, jejímž cílem bylo napsat program, který bude klasifikovat emaily jako spamy, nebo jako nezávadné, s co nejvyšší přesností. Filtr má možnost se naučit jak spam vypadá na testovací sadě emailů.

#pagebreak()

== Trénování filtru
Dle specifikace začíná algoritmus trénování v metodě `train()`. Algoritmus se skládá ze dvou hlavních částí; spočítání tzv. flagů a spočítání tzv. limitů. Flagy jsou určité vlastnosti emailu, na jejichž základě se následně emaily klasifikují. Limity pak určují kolikrát je potřeba porušit nějakou takovou vlastnost, aby mohl být email díky ní klasifikován jako spam. 
\ \
Flagy, které se počítají z trénovací sady dat jsou:
- Headery, které se vyskytují pouze ve spamových emailech
- Domény odesílatele, které se vyskytují pouze ve spamových emailech
- Odkazy vyskytující se pouze ve spamových emailech
- Průměrný poměr velkých a malých písmen ve spamových emailech
- Průměrný výskyt předem zvolených znaků v nezávadných emailech společně se znaky vyskytujícími se pouze ve spamových emailech (výskyt = 0)
Další flagy zjištěné externími skripty, které tvoří statické seznamy:
- Často se vyskytující slova v předmětech spamových emailů
- Často se vyskytující fráze v předmětech spamových emailů
\
Poté se iteračně počítají limity k jednotlivým flagům. Nejdříve jsou pro každý spamový email zjištěny všechny flagy, které (a kolikrát) má. V každé iteraci se stanoví klasifikace všech emailů na základě jeho spočítaných flagů a aktuálních limitů (viz kapitola _Klasifikace emailů_). Poté je spočítána úspěšnost filtru, kdy pokud je úspěšnost nižší, než v minulé iteraci, limity jsou vráceny do stavu minulé iterace; v opačném případě je zaznamenán aktuální stav a každý limit je upraven přičtením náhodného čísla z `<-3; 3>` (limit musí být `>= 0`). Algoritmus iteruje 150×.

#pagebreak()

== Klasifikace emailů
Dle specifikace začíná algoritmus klasifikace v metodě `test()`. Ke klasifikaci je nejprve použita metoda na počítání flagů pro všechny emaily. Poté je vyhodnocena klasifikace všech těchto emailů stejným způsobem jako při počítání limitů. Za každé porušení limitu je emailu připočten jeden strike. Aby byl email považován za spam, musí porušit alespoň 1/3 všech limitů.
#v(1em)
#show raw.where(block: true): block.with(fill: luma(240), inset: 10pt, radius: 4pt)
```Python    
def is_spam(self, flags):
    strikes = 0
    for flag, value in flags.items():
        if value > self.limits[flag]:
            strikes += 1
    return strikes >= round(len(self.limits) / 3)
```


== Výsledky filtru
Filtr se nechová zcela deterministicky, ovšem dosahuje úspěšnosti průměrně *50,6 %* na testovacích sadách dat (viz graf). Na sadách dat automatického testování v BRUTE dosahuje úspešnosti průměrně *41,5 %*.
#v(1em)
#let graph_testdata() = {
  let data = (
    (38.44, 1),(53.96, 2),(60.29, 3),(39.67, 4),(29.05,5),(39.07,6),(47.63,7),(63.21,8),(27.85,9),(46.81,10),(58.03,11),(27.04,12),(66.3,13),(64.43,14),(64.43,15),(63.96,16),(63.34,17),(58.62,18),(36.32,19),(63.49,20)
  )
  let x_axis = axis(title: "Běh", min: 0, max:21, location: "bottom", show_markings: false)
  let y_axis = axis(title: "%", min: 0, max: 110, step: 10, location: "left", helper_lines: true)
  let pl = plot(axes: (x_axis, y_axis), data: data)
  bar_chart(pl, (100%, 33%), bar_width: 75%, caption: none)
}
#graph_testdata()

== Rozdělení práce
Pro sdílení kódu jsme používali GitHub repozitář. Postupně jsme přidávali nové funkcionality a navzájem je vylepšovali.
#v(1em)
#table(
  columns: (1fr, 1fr),
  inset: 10pt,
  align: horizon,
  table.header(align(center)[*Petr*], align(center)[*Martin*]),
  [
    - Analýza headerů emailu
    - Analýza výskytu znaků v emailu
    - Analýza poměru velkých a malých písmen v emailu
    - Algoritmus trénování filtru
    - Klasifikace emailů
    - Zpětná vazba
  ],
  [
    - Parsování emailu
    - Extrakce odkazů
    - Extrakce domén
    - Analýza výskytu slov a frází v předmětu emailu
    - Trénování filtru
    - Klasifikace emailů
    - Zpětná vazba
  ]
)

#pagebreak()

== Závěr
Podařilo se nám úspěšně splnit zadání úlohy. Úspěšnost samotného filtru není vždy stabilní a nedosahuje takových výsledků, jaké by umožňovaly jeho reálné využití. Vylepšením stávajících algoritmů a faktorů, na základě kterých se filtr rozhoduje, by se využitelným stát ovšem mohl. Některé změny by nejspíše nemusely být ani příliš velké.

Nicméně projekt dosáhl ve větší míře očekávání a byl velmi přínosný zejména prací v týmu, ale také v jazyce Python, se kterým jsme neměli ani jeden příliš velké zkušenosti.