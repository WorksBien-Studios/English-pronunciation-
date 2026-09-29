const { chromium } = require('/opt/node22/lib/node_modules/playwright');
(async () => {
  const b = await chromium.launch({ executablePath: '/opt/pw-browsers/chromium' });
  const svg = require('fs').readFileSync('app-icon.svg', 'utf8');
  const p = await b.newPage({ viewport: { width: 1024, height: 1024 } });
  await p.setContent(`<body style="margin:0">${svg}</body>`);
  await p.screenshot({ path: 'app-icon-1024.png', omitBackground: false });
  await b.close();
})();
