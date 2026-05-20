export default {
  // Meta / Layout
  'meta.description':
    'wiRedPanda - Et gratis, intuitivt værktøj til at lære og designe digitale logiske kredsløb.',

  // Navigation
  'nav.home': 'Hjem',
  'nav.about': 'Om',
  'nav.download': 'Download',
  'nav.tryout': 'Prøv online',
  'nav.github': 'GitHub',
  'nav.githubRepo': 'GitHub-repository',
  'nav.toggleMenu': 'Vis/skjul navigationsmenuen',

  // Hero
  'hero.logoAlt': 'wiRedPanda-logo',
  'hero.tagline': 'En sjov og nem måde at lære logiske kredsløb på',
  'hero.scrollDown': 'Rul ned',

  // Home page
  'home.title': 'Hjem',
  'home.intro':
    'wiRedPanda er et kraftfuldt og intuitivt værktøj, der er designet til at hjælpe studerende, undervisere og entusiaster på deres rejse med at lære om digitale kredsløb. Med en brugervenlig brugergrænseflade og avancerede funktioner forvandler wiRedPanda processen med at forstå og skabe kredsløb til en interaktiv og engagerende oplevelse.',
  'home.mainFeatures': 'Hovedfunktioner',
  'home.feature.simulation.title': 'Simulering i realtid',
  'home.feature.simulation.content':
    'Se dine kredsløb komme til live øjeblikkeligt, mens du justerer og eksperimenterer med forskellige konfigurationer.',
  'home.feature.interface.title': 'Intuitiv brugergrænseflade',
  'home.feature.interface.content':
    'Træk og slip elementer, tilslut ledninger og byg dine kredsløb enkelt og effektivt.',
  'home.feature.analysis.title': 'Detaljeret analyse',
  'home.feature.analysis.content':
    'Analyser dine kredsløb med beWavedDolphin-bølgeformsimulatoren, identificer og ret problemer.',
  'home.feature.crossplatform.title': 'Platformsuafhængig',
  'home.feature.crossplatform.content':
    'Tilgængelig til Windows, macOS og Linux, så du kan arbejde på dine projekter på enhver enhed.',
  'home.mission':
    'Vi mener, at teknologiundervisning skal være tilgængelig, engagerende og praktisk. Vores software er udviklet for at forenkle læringsprocessen og lade dig fokusere på det, der virkelig betyder noget: at forstå og skabe digitale kredsløb.',
  'home.screenshot1Alt': 'Skærmbillede af wiRedPanda-kredsløbseditoren',
  'home.screenshot2Alt': 'Skærmbillede af en logisk kredsløbssimulering i wiRedPanda',
  'home.demoLabel': 'wiRedPanda-demovideo',
  'home.cta':
    'Download wiRedPanda nu og begynd at udforske verden af digitale kredsløb. Prøv alle vores funktioner og se, hvordan vi kan hjælpe dig med at nå dine mål. Allerede i brug på Federal University of São Paulo, og måske snart også i din klasse for logiske kredsløb!',

  // About page
  'about.title': 'Om',
  'about.heading': 'Om projektet',
  'about.history':
    'Projektet blev født i 2015 med Dr. Fábio Cappabianco, Davi Morales, Lucas Lellis og Rodrigo Torres med det formål at have et softwareprogram, der kan gøre det nemt at lære logiske kredsløb, 100% gratis og platformsuafhængigt. Bygget til at være nemt at bruge, bruges softwaren i dag til at undervise i digitale kredsløb på UNIFESP samt i andre universitetsprojekter, der underviser gymnasieelever i digitale logik i São José dos Campos offentlige skoler.',
  'about.campusAlt': 'Unifesp campus',
  'about.contribute':
    'Vi er i aktiv udvikling, så hvis du vil rapportere fejl eller dele nye idéer, åbn en diskussion på GitHub! Med din hjælp kan vi bestemt forbedre softwaren.',
  'about.team': 'wiRedPanda-teamet',
  'about.formerDevs': 'Tidligere udviklere',

  // Download page
  'download.title': 'Download',
  'download.heading': 'Download softwaren',
  'download.fetching': 'Henter oplysninger om den seneste version...',
  'download.errorHeading': 'Kunne ikke indlæse de seneste downloadlinks:',
  'download.errorVisit': 'Besøg venligst',
  'download.errorVisitLink': 'GitHub Releases-siden',
  'download.errorVisitSuffix': 'direkte.',
  'download.windows': 'Windows',
  'download.linux': 'Linux',
  'download.macos': 'Mac OS',
  'download.otherOptions': 'Andre muligheder',
  'download.packageManagers':
    'Du kan også installere wiRedPanda ved hjælp af pakkehåndteringer på nogle Linux-distributioner. Disse pakker er muligvis ikke altid den absolut nyeste version, der findes på GitHub releases.',
  'download.allReleases':
    'For alle versioner, inklusive ældre versioner eller forudversioner, besøg',
  'download.officialGithub': 'den officielle GitHub Releases-side',
  'download.copiedToClipboard': 'Kopieret til udklipsholder!',
  'download.copyCommand': 'Kopier installationskommando til {platform}',
  'download.versionText': 'Her kan du downloade de binære filer for wiRedPanda {version} til:',
  'download.errorFetching': 'Fejl ved hentning af den seneste version: {status} {statusText}.',
  'download.errorNotFound': 'Ingen versioner fundet for {owner}/{repo}.',
  'download.errorRateLimit':
    'GitHub API-hastighedsgrænsen er sandsynligvis overskredet. Prøv igen senere.',
  'download.errorConnection':
    'Kunne ikke oprette forbindelse til GitHub API eller fortolke svaret. Tjek din internetforbindelse, eller prøv igen senere.',
  'download.x86_64': 'x86_64',
  'download.arm64': 'ARM64',
  'download.universal': 'Universel',
  'download.platformArch': 'Arch-baseret',
  'download.platformDebian': 'Debian-baseret',

  // 404 page
  '404.title': '404',
  '404.heading': 'Ups!',
  '404.message':
    "Det ser ud til, at du er havnet i 'don't care'-området på Karnaugh-kortet, men bare rolig!",
  '404.submessage':
    'Vi har allerede forenklet processen — gå bare tilbage til hovedsiden, så går det nok.',
  '404.backHome': 'Tilbage til forsiden',

  // Footer
  'footer.unifespAlt': 'UNIFESP-logo',
  'footer.copyright': 'wiRedPanda-teamet',
  'footer.gibisAlt': 'GIBIS-logo',

  // Theme toggle
  'theme.toggle': 'Skift mørk tilstand',

  // Language switcher
  'lang.switchLanguage': 'Skift sprog',
} as const;
