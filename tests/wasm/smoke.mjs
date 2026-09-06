// Run after a WASM build: node tests/wasm/smoke.mjs build-wasm
import http from 'node:http';
import fs from 'node:fs/promises';
import path from 'node:path';
import assert from 'node:assert/strict';
import {installShowcaseProbe, exerciseShowcase} from './showcase.mjs';
const { chromium, firefox, webkit } = await import(process.env.PLAYWRIGHT_MODULE || 'playwright');
const root = path.resolve(process.argv[2] || 'build-wasm');
const engine = process.env.WUI_BROWSER || 'chromium';
const server = http.createServer(async (req, res) => {
    try {
        const file = path.resolve(root, '.' + decodeURIComponent(new URL(req.url, 'http://localhost').pathname));
        if (!file.startsWith(root + path.sep)) throw new Error('Invalid path');
        const data = await fs.readFile(file);
        res.writeHead(200, {'Content-Type': ({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.png':'image/png'})[path.extname(file)] || 'application/octet-stream'});
        res.end(data);
    } catch { res.writeHead(404); res.end(); }
});
await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
const url = `http://127.0.0.1:${server.address().port}`;
let browser;
try {
    browser = await ({chromium,firefox,webkit})[engine].launch({headless:true});
    for (const example of ['hello_world','simple','demo']) {
        const page = await browser.newPage({viewport:{width:1280,height:900},deviceScaleFactor:2});
        const errors=[];
        page.on('pageerror', e => errors.push(e.message));
        page.on('console', e => { if(e.type()==='error') errors.push(e.text()); });
        if(example==='demo') await installShowcaseProbe(page);
        await page.goto(`${url}/examples/${example}/${example}.html`);
        await page.waitForFunction(() => globalThis.Module?.wui?.windows.size > 0);
        await page.waitForFunction(() => [...Module.wui.images.values()].every(i => i.complete && i.naturalWidth));
        await page.waitForFunction(() => [...Module.wui.windows.values()].some(w => {
            const pixels=w.canvas.getContext('2d').getImageData(0,0,w.canvas.width,w.canvas.height).data;
            for(let i=3;i<pixels.length;i+=4) if(pixels[i]) return true;
            return false;
        }));
        await page.screenshot({path:path.join(root,`${example}-${engine}.png`)});
        if(example==='demo') await exerciseShowcase(page,root,engine);
        const windows=await page.evaluate(() => [...Module.wui.windows.values()].map(w=>({id:w.id,width:w.width,height:w.height,title:w.title})));
        assert.deepEqual(errors,[],`${example}: browser errors`);
        console.log(JSON.stringify({example,engine,windows,errors}));
        await page.close();
    }
    if (await fs.stat(path.join(root,'tests/wasm/wui_wasm_test.html')).catch(()=>null)) {
        const page=await browser.newPage({viewport:{width:1280,height:900},deviceScaleFactor:2});
        const errors=[];page.on('pageerror',e=>errors.push(e.message));
        await page.goto(`${url}/tests/wasm/wui_wasm_test.html`);
        await page.waitForFunction(()=>globalThis.Module?.wui?.windows.size===1 && Module._test_value(1)===1);
        const value=which=>page.evaluate(which=>Module._test_value(which),which);
        const text=which=>page.evaluate(which=>Module.ccall('test_text','string',['number'],[which]),which);
        const action=which=>page.evaluate(which=>Module._test_action(which),which);
        const canvas=page.locator('.wui-canvas').first();
        const click=async(x,y)=>canvas.click({position:{x,y}});
        assert.equal(await page.evaluate(()=>Module._test_pixels()),1,'RGBA / 2x / clipped offscreen blit');
        await click(70,220);assert.equal(await value(0),1,'First click reaches control; main stack remains alive');
        const bounds=await canvas.boundingBox();
        await page.mouse.move(bounds.x+357,bounds.y+100);
        await page.waitForFunction(()=>Module._test_value(7)>0);
        await page.mouse.move(bounds.x+357,bounds.y+252);await page.mouse.down();
        await page.waitForFunction(()=>Module._test_value(8)>0);await page.mouse.up();
        await click(60,80);await page.keyboard.insertText('Привет 🌍');assert.equal(await text(0),'Привет 🌍');
        await page.keyboard.press('Backspace');assert.equal(await text(0),'Привет ');
        await page.keyboard.press('Control+a');await page.keyboard.insertText('hello');assert.equal(await text(0),'hello');
        await page.keyboard.press('Tab');assert.equal(await value(4),1);
        await page.keyboard.insertText('line 1');await page.keyboard.press('Enter');await page.keyboard.insertText('line 2');
        assert.equal(await text(1),'line 1\nline 2');
        await page.keyboard.press('Shift+Tab');assert.equal(await value(3),1);
        await page.keyboard.press('End');
        await page.evaluate(()=>{
            const e=document.activeElement;
            e.dispatchEvent(new CompositionEvent('compositionstart',{bubbles:true}));
            e.value='日本語';e.dispatchEvent(new InputEvent('input',{bubbles:true,inputType:'insertCompositionText',data:'日本語',isComposing:true}));
        });
        assert.equal(await text(0),'hello','Marked text is not committed early');
        await page.evaluate(()=>{
            const e=document.activeElement;e.dispatchEvent(new CompositionEvent('compositionend',{bubbles:true,data:'日本語'}));
            e.value='日本語';e.dispatchEvent(new InputEvent('input',{bubbles:true,inputType:'insertFromComposition',data:'日本語'}));
        });
        assert.equal(await text(0),'hello日本語','Composition commits once');
        await page.keyboard.press('Control+a');
        const copied=await page.evaluate(()=>{const event=new ClipboardEvent('copy',{bubbles:true,cancelable:true,clipboardData:new DataTransfer()});document.activeElement.dispatchEvent(event);return event.clipboardData.getData('text/plain');});
        assert.equal(copied,'hello日本語');
        await page.evaluate(()=>{const event=new ClipboardEvent('paste',{bubbles:true,cancelable:true,clipboardData:new DataTransfer()});event.clipboardData.setData('text/plain','Вставка 🦊');document.activeElement.dispatchEvent(event);});
        assert.equal(await text(0),'Вставка 🦊');
        await action(0);await page.waitForFunction(()=>Module._test_value(2)===1);
        assert.equal(await value(1),1,'Timer can destroy itself');
        await action(6);await page.keyboard.insertText('BLOCKED');assert.equal(await text(0),'Вставка 🦊','Modal routes keyboard away from background');
        await page.keyboard.press('Enter');await page.waitForFunction(()=>Module._test_value(6)===1);
        await action(7);await page.waitForFunction(()=>Module.wui.windows.size===2);
        await page.keyboard.press('Enter');await page.waitForFunction(()=>Module.wui.windows.size===1 && Module._test_value(6)===2);
        // Hold a drag outside the input while its selection timer runs.
        await action(8);
        const selection=()=>page.evaluate(()=>{
            const e=new ClipboardEvent('copy',{bubbles:true,cancelable:true,clipboardData:new DataTransfer()});
            document.activeElement.dispatchEvent(e);return e.clipboardData.getData('text/plain');
        });
        await page.mouse.move(bounds.x+25,bounds.y+126);await page.mouse.down();
        await page.mouse.move(bounds.x+270,bounds.y+190);
        await page.waitForFunction(()=>{
            const e=new ClipboardEvent('copy',{bubbles:true,cancelable:true,clipboardData:new DataTransfer()});
            document.activeElement.dispatchEvent(e);return e.clipboardData.getData('text/plain').endsWith('z');
        });
        await page.mouse.up();
        assert.equal(await selection(),'a'+'\n'.repeat(30)+'z','Drag autoscroll reaches final line');
        await page.mouse.move(bounds.x+25,bounds.y+126);
        assert.equal(await selection(),'a'+'\n'.repeat(30)+'z','Released drag keeps its selection');
        await page.mouse.move(bounds.x+25,bounds.y+80);await page.mouse.down();
        await page.mouse.move(bounds.x+325,bounds.y+80);
        await page.waitForTimeout(350);
        assert.equal(await selection(),'я','Autoscroll stops at final Unicode character');
        await page.mouse.up();
        await action(9);
        await page.keyboard.press('End');
        await page.keyboard.press('Shift+ArrowRight');await page.keyboard.press('Shift+ArrowRight');
        assert.equal(await selection(),'\n\n','Empty line and adjacent newline can be selected');
        await page.waitForFunction(()=>{
            const c=document.querySelector('.wui-canvas'),scale=c.width/c.getBoundingClientRect().width;
            return Array.from(c.getContext('2d').getImageData(27*scale,150*scale,1,1).data).join(',')==='38,79,120,255';
        });
        await action(1);await page.waitForFunction(()=>[...Module.wui.windows.values()][0].width===1280);
        await page.setViewportSize({width:1100,height:800});await page.waitForFunction(()=>[...Module.wui.windows.values()][0].width===1100);
        await action(2);await page.waitForFunction(()=>[...Module.wui.windows.values()][0].width===500);
        const box=await canvas.boundingBox();
        await page.mouse.move(box.x+box.width-2,box.y+box.height-2);await page.mouse.down();
        await page.mouse.move(box.x+box.width+48,box.y+box.height+28,{steps:4});await page.mouse.up();
        await page.waitForFunction(()=>[...Module.wui.windows.values()][0].width===550);
        await action(3);await page.locator('.wui-restore').click();await page.waitForFunction(()=>![...Module.wui.windows.values()][0].root.hidden);
        await action(4);assert.equal(await value(5),0,'Close callback can veto');
        await page.screenshot({path:path.join(root,`integration-${engine}.png`)});
        await action(5);await page.waitForFunction(()=>Module.wuiTestReturned===true);
        assert.equal(await value(5),1);assert.equal(await page.evaluate(()=>Module.wui.windows.size),0);
        assert.deepEqual(errors,[]);
        console.log(JSON.stringify({integration:'passed',engine}));await page.close();
    }
} finally { await browser?.close(); await new Promise(resolve=>server.close(resolve)); }
