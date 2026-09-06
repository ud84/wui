// Observe rendered text instead of adding test-only exports to the application.
import path from 'node:path';
export async function installShowcaseProbe(page) {
    await page.addInitScript(() => {
        globalThis.showcaseText = [];
        const original = CanvasRenderingContext2D.prototype.fillText;
        CanvasRenderingContext2D.prototype.fillText = function(text, ...args) {
            showcaseText.push(String(text));
            if(showcaseText.length > 10000) showcaseText.splice(0,5000);
            return original.call(this,text,...args);
        };
    });
}
export async function exerciseShowcase(page, root, engine) {
    const canvas = page.locator('.wui-canvas').first();
    const click = (x,y) => canvas.click({position:{x,y}});
    const clear = () => page.evaluate(() => { showcaseText=[]; });
    const seen = text => page.waitForFunction(text => showcaseText.some(t=>t.includes(text)),text);
    const tab = async i => { await clear(); await click(74+i*108,56); };
    const shot = name => page.screenshot({path:path.join(root,`showcase-${name}-${engine}.png`)});
    await seen('Small library.');
    await clear(); await click(440,464);
    await page.waitForFunction(()=>showcaseText.some(t=>t.startsWith('Amplitude /') && t!=='Amplitude / 60%'));
    const amplitude=await page.evaluate(()=>showcaseText.filter(t=>t.startsWith('Amplitude /')).at(-1));
    await click(630,416); await click(600,482); // square wave
    await shot('overview');
    await tab(2); await seen('eight personalities');
    await click(120,260); await seen('1 clicks');
    await click(120,260); await seen('2 clicks');
    await click(600,318); await click(300,260); await seen('Previously disabled');
    await shot('buttons');
    await tab(3); await seen('Text editing');
    await click(130,278); await page.keyboard.press('ControlOrMeta+A');
    await page.keyboard.insertText('Showcase user'); await seen('Name / Showcase user');
    await click(470,514); await seen('1 lines');
    await click(660,514); await seen('Unicode:');
    await shot('inputs');
    await tab(4); await seen('120 sample projects');
    await click(100,192); await page.keyboard.insertText('Signal'); await seen('20 sample projects');
    await click(80,306); await seen('Selected / Signal Lab');
    await clear(); await click(100,192); await page.keyboard.press('ControlOrMeta+A'); await page.keyboard.insertText('no-such-project');
    await seen('0 sample projects');
    await click(660,514); await seen('120 sample projects');
    await shot('lists');
    await tab(5); await seen('Small interactions matter');
    await click(140,344); await seen('New project'); await shot('menu-open');
    await click(110,389); await seen('Menu / New project');
    await click(620,364); await seen('A WUI tooltip');
    await shot('menus');
    await tab(6); await seen('Make room');
    const bounds = await canvas.boundingBox();
    const before = await canvas.evaluate(c=>c.toDataURL());
    await page.mouse.move(bounds.x+310,bounds.y+310); await page.mouse.down();
    await page.mouse.move(bounds.x+430,bounds.y+310,{steps:8}); await page.mouse.up();
    await page.waitForFunction(before=>document.querySelector('.wui-canvas').toDataURL()!==before,before);
    await shot('layout');
    await tab(1); await seen('Windows with');
    await click(170,310); await seen('This dialog is drawn');
    await shot('dialog');
    await page.keyboard.press('Enter'); await seen('Dialog / OK');
    await click(580,440); await page.waitForFunction(()=>Module.wui.windows.size===2);
    const child=page.locator('.wui-canvas').last();
    await child.click({position:{x:400,y:15}}); await page.waitForFunction(()=>Module.wui.windows.size===1);
    // Edits survive navigation and theme changes.
    await tab(3); await seen('Showcase user');
    await click(653,15); await seen('Theme / light'); await shot('light');
    await tab(0); await seen(amplitude);
    await shot('overview-light');
    // Resizing preserves navigation, split layout and control bounds.
    await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];Module._wui_position(w.id,w.x,w.y,1000,700);});
    await click(910,56); await seen('Make room'); await shot('resized');
    console.log(`PASS: showcase interactions (${engine})`);
}
