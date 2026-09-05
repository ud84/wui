// Copyright (c) 2026 Intent Garden Org. Boost Software License 1.0.
// Browser resources are scoped to this Emscripten Module.
Module.wui = {
    next: 1, z: 1, windows: new Map(), surfaces: new Map(), images: new Map(),
    clipboard: '', clipboardEvent: false, measuring: null,
    host: function() {
        var host = Module['wuiContainer'] || document.getElementById('wui');
        if (!host) throw new Error('WUI needs a #wui container or Module.wuiContainer');
        return host;
    },
    color: function(c) { return 'rgba(' + ((c >>> 16) & 255) + ',' + ((c >>> 8) & 255) + ',' + (c & 255) + ',' + (1 - (c >>> 24) / 255) + ')'; },
    surface: function(width, height, scale) {
        var c = document.createElement('canvas'); c.width = Math.ceil(width * scale); c.height = Math.ceil(height * scale);
        var ctx = c.getContext('2d'); if (!ctx) return 0;
        ctx.scale(scale, scale); ctx.wuiScale = scale;
        var id = this.next++; this.surfaces.set(id, ctx); return id;
    },
    draw: function(id, op, x, y, w, h, color, fill, line, radius) {
        var c = this.surfaces.get(id); if (!c) return;
        c.save(); c.fillStyle = this.color(color); c.strokeStyle = c.fillStyle; c.lineWidth = Math.max(1, line);
        if (op === 0) { c.clearRect(x,y,w,h); c.fillRect(x,y,w,h); }
        if (op === 1 && line) { c.beginPath(); c.moveTo(x+0.5,y+0.5); c.lineTo(w+0.5,h+0.5); c.stroke(); }
        if (op === 2) c.fillRect(x,y,w,h);
        if (op === 3 && w > line && h > line) {
            c.beginPath(); c.roundRect(x+line/2,y+line/2,w-line,h-line,Math.max(0,Math.min(radius,(w-line)/2,(h-line)/2)));
            c.fillStyle = this.color(fill); c.fill(); if (line) c.stroke();
        }
        c.restore();
    },
    text: function(id, text, name, size, decoration, x, y, color) {
        if (!this.measuring) this.measuring = document.createElement('canvas').getContext('2d');
        var c = this.surfaces.get(id) || this.measuring;
        size = Math.max(1, size);
        // WUI treats size as line height. Normalize the em to the font metrics.
        var family = JSON.stringify(name || 'sans-serif') + ', sans-serif';
        var prefix = ((decoration & 2) ? 'italic ' : '') + ((decoration & 1) ? 'bold ' : '');
        c.save(); c.font = prefix + size + 'px ' + family;
        var metrics = c.measureText('Mg');
        var ascent = metrics.fontBoundingBoxAscent || size * 0.8;
        var descent = metrics.fontBoundingBoxDescent || size * 0.2;
        var ratio = size / (ascent + descent);
        c.font = prefix + (size * ratio) + 'px ' + family;
        var width = c.measureText(text).width;
        if (id) {
            c.textBaseline = 'alphabetic'; c.fillStyle = this.color(color); c.fillText(text,x,y+ascent*ratio);
            c.strokeStyle = c.fillStyle; c.lineWidth = 1;
            if (decoration & 4) { c.beginPath(); c.moveTo(x,y+ascent*ratio+1); c.lineTo(x+width,y+ascent*ratio+1); c.stroke(); }
            if (decoration & 8) { c.beginPath(); c.moveTo(x,y+size/2); c.lineTo(x+width,y+size/2); c.stroke(); }
        }
        c.restore(); return width;
    },
    blit: function(id, src, x, y, width, height, sx, sy) {
        var c = this.surfaces.get(id), s = this.surfaces.get(src); if (!c || !s || width <= 0 || height <= 0) return;
        c.save(); c.beginPath(); c.rect(x,y,width,height); c.clip();
        c.drawImage(s.canvas,x-sx,y-sy,s.canvas.width/s.wuiScale,s.canvas.height/s.wuiScale); c.restore();
    },
    pixels: function(id, ptr, x, y, width, height, sx, sy) {
        var c = this.surfaces.get(id); if (!c) return;
        var bytes = HEAPU8.subarray(ptr,ptr+width*height*4), rgba = new Uint8ClampedArray(bytes.length);
        for(var n=0;n<bytes.length;n+=4) { rgba[n]=bytes[n+2]; rgba[n+1]=bytes[n+1]; rgba[n+2]=bytes[n]; rgba[n+3]=255; }
        var tmp=document.createElement('canvas'); tmp.width=width;tmp.height=height;
        tmp.getContext('2d').putImageData(new ImageData(rgba,width,height),0,0);
        c.save();c.beginPath();c.rect(x,y,width,height);c.clip();c.drawImage(tmp,x-sx,y-sy);c.restore();
    },
    image: function(bytes) {
        var id = this.next++, image = new Image(), self = this;
        var url = URL.createObjectURL(new Blob([bytes])); this.images.set(id,image);
        image.onload = function() { URL.revokeObjectURL(url); self.windows.forEach(function(w) { self.invalidate(w.id); }); };
        image.onerror = function() { URL.revokeObjectURL(url); console.error('WUI: image decoding failed'); };
        image.src = url; return id;
    },
    present: function(id, windowId) {
        var c = this.surfaces.get(id), w = this.windows.get(windowId); if (!c || !w) return;
        var ctx = w.canvas.getContext('2d');ctx.clearRect(0,0,w.canvas.width,w.canvas.height);
        ctx.drawImage(c.canvas,0,0,w.canvas.width,w.canvas.height);
    },
    invalidate: function(id) {
        var w = this.windows.get(id); if(!w || w.pending) return;
        w.pending = requestAnimationFrame(function() { w.pending=0; if(Module.wui.windows.has(id)) Module._wui_paint(id); });
    },
    create: function(x,y,width,height) {
        if(!this.hostObserver) {
            this.hostObserver=new ResizeObserver(function() {
                Module.wui.windows.forEach(function(w) { if(Module._wui_viewport) Module._wui_viewport(w.id); });
            });
            this.hostObserver.observe(this.host());
        }
        var self=this, id=this.next++, root=document.createElement('div'), canvas=document.createElement('canvas'), editor=document.createElement('textarea');
        root.className='wui-window'; canvas.className='wui-canvas'; editor.className='wui-input';
        canvas.setAttribute('role','application'); editor.setAttribute('aria-label','WUI text input');
        editor.setAttribute('autocomplete','off');editor.setAttribute('autocapitalize','off');editor.spellcheck=false;
        root.append(canvas,editor); this.host().appendChild(root);
        var w={id:id,root:root,canvas:canvas,editor:editor,x:x,y:y,width:width,height:height,minw:40,minh:40,pending:0,composing:false,input:false};
        this.windows.set(id,w); this.position(id,x,y,width,height); root.style.zIndex=++this.z;
        var coords=function(e) { var r=canvas.getBoundingClientRect();return [Math.round((e.clientX-r.left)*w.width/r.width),Math.round((e.clientY-r.top)*w.height/r.height)]; };
        var mouse=function(e,type,delta) { var p=coords(e);Module._wui_mouse(id,type,p[0],p[1],delta||0); };
        canvas.addEventListener('contextmenu',function(e){ e.preventDefault(); });
        canvas.addEventListener('pointerdown',function(e) {
            var p=coords(e), flags=Module._wui_flags(id,p[0],p[1]); if(!(flags&1)) return;
            e.preventDefault();root.style.zIndex=++self.z; editor.focus({preventScroll:true});canvas.setPointerCapture(e.pointerId);
            var edges=0;
            if(e.button===0 && (flags&2)) { if(p[0]<5) edges|=1;if(p[0]>w.width-5) edges|=2;if(p[1]<5) edges|=4;if(p[1]>w.height-5) edges|=8; }
            if(e.button===0 && (edges || (p[1]<30 && (flags&4) && !(flags&8)))) {
                w.drag={x:e.clientX,y:e.clientY,left:w.x,top:w.y,width:w.width,height:w.height,edges:edges};return;
            }
            mouse(e,e.button===2?3:e.button===1?5:7);
        });
        canvas.addEventListener('pointermove',function(e) {
            if(w.drag) {
                var d=w.drag,dx=e.clientX-d.x,dy=e.clientY-d.y,x=d.left,y=d.top,width=d.width,height=d.height;
                if(!d.edges) { x+=dx;y+=dy; }
                else {
                    if(d.edges&1) { width=Math.max(w.minw,d.width-dx);x=d.left+d.width-width; }
                    if(d.edges&2) width=Math.max(w.minw,d.width+dx);
                    if(d.edges&4) { height=Math.max(w.minh,d.height-dy);y=d.top+d.height-height; }
                    if(d.edges&8) height=Math.max(w.minh,d.height+dy);
                }
                Module._wui_position(id,Math.round(x),Math.round(y),Math.round(width),Math.round(height));
            } else mouse(e,0);
        });
        canvas.addEventListener('pointerup',function(e) { if(w.drag) w.drag=null;else mouse(e,e.button===2?4:e.button===1?6:8);if(canvas.hasPointerCapture(e.pointerId)) canvas.releasePointerCapture(e.pointerId); });
        canvas.addEventListener('pointercancel',function(e) { w.drag=null;mouse(e,8);mouse(e,2); });
        canvas.addEventListener('pointerleave',function(e) { if(!canvas.hasPointerCapture(e.pointerId)) mouse(e,2); });
        canvas.addEventListener('dblclick',function(e) { mouse(e,9); });
        canvas.addEventListener('wheel',function(e) { if(!(Module._wui_flags(id,0,0)&1)) return;e.preventDefault();mouse(e,10,Math.round(-e.deltaY*(e.deltaMode===1?40:e.deltaMode===2?w.height:4))); },{passive:false});
        editor.addEventListener('focus',function() { Module._wui_focus(id,1); });
        editor.addEventListener('blur',function() { Module._wui_focus(id,0); });
        var codes={Tab:9,Enter:13,Escape:27,Backspace:8,Delete:46,End:35,Home:36,PageUp:33,PageDown:34,ArrowUp:38,ArrowDown:40,ArrowLeft:37,ArrowRight:39,Shift:16};
        editor.addEventListener('keydown',function(e) {
            if(e.isComposing || w.composing || e.keyCode===229) return;
            var key=e.key.toLowerCase();
            if((e.ctrlKey || e.metaKey) && !e.altKey) {
                if(key==='a') { e.preventDefault();Module._wui_key(id,2,1,162); }
                // Native clipboard events carry the actual clipboard contents.
                return;
            }
            var code=codes[e.key];
            if(code) { e.preventDefault();Module._wui_key(id,0,code,e.shiftKey?16:e.altKey?18:0); }
        });
        editor.addEventListener('keyup',function(e) { if(!w.composing) Module._wui_key(id,1,codes[e.key]||0,e.shiftKey?16:0); });
        var text=function(value) { if(value) Module.ccall('wui_text',null,['number','string'],[id,value]); };
        editor.addEventListener('beforeinput',function(e) {
            if(w.composing || e.isComposing) return;
            if(e.inputType==='deleteContentBackward' || e.inputType==='deleteContentForward') {
                e.preventDefault();Module._wui_key(id,0,e.inputType==='deleteContentBackward'?8:46,0);
            } else if(e.inputType==='insertLineBreak' || e.inputType==='insertParagraph') {
                e.preventDefault();Module._wui_key(id,0,13,0);
            }
        });
        editor.addEventListener('input',function(e) {
            if(w.composing || e.isComposing) return;
            // Some browsers send a final input after compositionend.
            if(w.committed && (e.inputType==='insertFromComposition' || editor.value===w.committed)) { w.committed='';editor.value='';return; }
            text(editor.value);editor.value='';
        });
        editor.addEventListener('compositionstart',function() { w.composing=true;w.committed='';editor.value='';editor.classList.add('wui-composing'); });
        editor.addEventListener('compositionend',function(e) {
            w.composing=false;w.committed=e.data || '';text(w.committed);editor.value='';editor.classList.remove('wui-composing');
            setTimeout(function(){w.committed='';},0);
        });
        editor.addEventListener('paste',function(e) {
            e.preventDefault();Module.ccall('wui_clipboard',null,['string'],[e.clipboardData.getData('text/plain')]);Module._wui_key(id,2,22,162);editor.value='';
        });
        ['copy','cut'].forEach(function(kind) { editor.addEventListener(kind,function(e) {
            e.preventDefault();self.clipboard='';self.clipboardEvent=true;
            try { Module._wui_key(id,2,kind==='copy'?3:24,162);e.clipboardData.setData('text/plain',self.clipboard); }
            finally { self.clipboardEvent=false; }
        }); });
        w.observer=new ResizeObserver(function() {
            if(!self.windows.has(id)) return;
            var r=root.getBoundingClientRect(), nw=Math.round(r.width), nh=Math.round(r.height);
            if(nw>0 && nh>0) { w.width=nw;w.height=nh;self.resizeCanvas(w);Module._wui_resize(id,nw,nh,Math.max(1,devicePixelRatio||1)); }
        });w.observer.observe(root);
        return id;
    },
    resizeCanvas: function(w) {
        var scale=Math.max(1,devicePixelRatio||1),width=Math.ceil(w.width*scale),height=Math.ceil(w.height*scale);
        if(w.canvas.width!==width) w.canvas.width=width;if(w.canvas.height!==height) w.canvas.height=height;
    },
    position: function(id,x,y,width,height) {
        var w=this.windows.get(id);if(!w) return;
        w.x=x;w.y=y;w.width=width;w.height=height;
        Object.assign(w.root.style,{left:x+'px',top:y+'px',width:width+'px',height:height+'px'});this.resizeCanvas(w);this.invalidate(id);
    },
    show: function(id,visible) {
        var w=this.windows.get(id);if(!w) return;
        w.root.hidden=!visible;
        if(visible) { if(w.restore) {w.restore.remove();w.restore=null;}w.root.style.zIndex=++this.z;w.editor.focus({preventScroll:true});this.invalidate(id); }
    },
    minimize: function(id) {
        var w=this.windows.get(id);if(!w) return;w.root.hidden=true;
        if(!w.restore) { var button=document.createElement('button');button.className='wui-restore';button.textContent=w.title||'Restore';
            button.addEventListener('click',function(){Module._wui_restore(id);});this.host().appendChild(button);w.restore=button; }
    },
    close: function(id) {
        var w=this.windows.get(id);if(!w) return;
        this.windows.delete(id);if(w.pending) cancelAnimationFrame(w.pending);w.observer.disconnect();w.root.remove();if(w.restore) w.restore.remove();
    }
};
window.addEventListener('resize',function() {
    Module.wui.windows.forEach(function(w) { Module.wui.resizeCanvas(w);if(Module._wui_resize) Module._wui_resize(w.id,w.width,w.height,Math.max(1,devicePixelRatio||1)); });
});
