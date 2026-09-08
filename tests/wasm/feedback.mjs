import assert from 'node:assert/strict';

// Regressions reported on Habr: test actual canvas pixels and browser focus,
// including an initially populated input (set_text used to hide the wheel bug).
export async function exerciseFeedback(page) {
    const canvas=page.locator('.wui-canvas').first();
    const click=(x,y)=>canvas.click({position:{x,y}});
    const settle=()=>page.evaluate(()=>new Promise(resolve=>requestAnimationFrame(()=>requestAnimationFrame(resolve))));
    const pixels=rect=>canvas.evaluate((c,[x,y,w,h])=>{
        const scale=c.width/c.getBoundingClientRect().width;
        return Array.from(c.getContext('2d').getImageData(x*scale,y*scale,w*scale,h*scale).data).join(',');
    },rect);
    assert.equal(await page.evaluate(()=>document.activeElement.tagName),'DIV','Opening demo does not focus text entry');
    await click(70,110);
    assert.equal(await page.evaluate(()=>document.activeElement.tagName),'DIV','Background does not open a text editor');
    await click(74+3*108,56);await settle();
    for(const rect of [[48,250,140,28],[48,378,100,28],[48,490,290,28]]) {
        const box=await canvas.boundingBox();
        await page.mouse.move(box.x+rect[0]+10,box.y+rect[1]+10);await settle();
        const before=await pixels(rect);
        for(let i=0;i<8;i++) await page.mouse.wheel(0,120);
        await page.waitForTimeout(150);await settle();
        assert.equal(await pixels(rect),before,'Wheel cannot hide single-line/read-only text');
    }
    await click(100,505);await settle();
    assert.equal(await page.evaluate(()=>document.activeElement.tagName),'DIV','Read-only text does not request a keyboard');
    const readonly=await pixels([48,490,290,28]);
    for(let i=0;i<4;i++) {await page.waitForTimeout(200);assert.equal(await pixels([48,490,290,28]),readonly,'Read-only caret never blinks');}
    await click(100,264);
    assert.equal(await page.evaluate(()=>document.activeElement.tagName),'TEXTAREA','Editable input requests text entry');
    // Same physical key, Russian layout. The clipboard shortcuts already use native events.
    await page.locator('.wui-input').first().dispatchEvent('keydown',{key:'ф',code:'KeyA',ctrlKey:true,bubbles:true});
    await page.keyboard.insertText('Layout independent');
    await page.waitForFunction(()=>showcaseText.some(t=>t==='Layout independent'));
    await page.locator('.wui-input').first().dispatchEvent('keydown',{key:'ф',code:'KeyA',metaKey:true,bubbles:true});
    await page.keyboard.insertText('Ada Lovelace');
    await click(74,56);await settle();
    // Dragging across -1 must not invoke the API's window-centering sentinel.
    await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];Module._wui_position(w.id,40,40,800,600);});await settle();
    const box=await canvas.boundingBox();
    await page.mouse.move(box.x+100,box.y+15);await page.mouse.down();
    await page.mouse.move(box.x+59,box.y-26);await settle();
    assert.deepEqual(await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];return [w.x,w.y];}),[-1,-1]);
    await page.mouse.move(box.x+58,box.y-27);await settle();
    assert.deepEqual(await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];return [w.x,w.y];}),[-2,-2]);
    await page.mouse.up();
    await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];Module._wui_position(w.id,40,40,800,600);});await settle();
    for(let i=0;i<3;i++) {
        await click(735,15);await settle();
        assert.ok(await page.evaluate(()=>{const w=[...Module.wui.windows.values()][0];return !w.root.hidden && w.width===Module.wui.host().clientWidth && w.height===Module.wui.host().clientHeight;}),'Maximize fills host');
        assert.ok((await pixels([30,100,1,1])).split(',').some((v,i)=>i%4===3 && Number(v)>0),'Maximized window remains painted');
        const width=await canvas.evaluate(c=>c.getBoundingClientRect().width);
        await click(width-65,15);await settle();
    }
    console.log('PASS: wheel, read-only, keyboard layout, focus, edge dragging and maximize regressions');
}

export async function exerciseTouchFocus(page) {
    await page.waitForFunction(()=>globalThis.Module?.wui?.windows.size>0);
    const canvas=page.locator('.wui-canvas').first();
    const tap=async(x,y)=>{const r=await canvas.boundingBox();await page.touchscreen.tap(r.x+x,r.y+y);};
    const active=()=>page.evaluate(()=>document.activeElement.tagName);
    assert.equal(await active(),'DIV','Touch page opens without focusing editable text');
    await tap(70,110);assert.equal(await active(),'DIV');
    await tap(398,56); // Inputs
    await tap(100,505);assert.equal(await active(),'DIV','Read-only tap keeps software keyboard closed');
    await tap(100,264);assert.equal(await active(),'TEXTAREA','Editable tap activates text entry');
    await page.keyboard.insertText('X');
    await tap(74,56);assert.equal(await active(),'DIV','Navigation leaves text entry');
    console.log('PASS: touch focus requests text entry only for editable input');
}
