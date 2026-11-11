// ============ КОНФИГ ============
const API = 'http://localhost:8080';

const TAG_COLORS = [
  '106, 86, 220',
  '65, 125, 177',
  '250, 133, 55',
  '49, 210, 57',
  '106, 166, 255',
];

function tagColor(name) {
  let h = 0;
  for (let i = 0; i < name.length; i++) h = (h * 31 + name.charCodeAt(i)) | 0;
  return TAG_COLORS[Math.abs(h) % TAG_COLORS.length];
}

function esc(s) {
  return String(s ?? '').replace(/[&<>"']/g, c => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;',
  }[c]));
}

function fmtDate(iso) {
  try {
    return new Date(iso).toLocaleString('ru-RU',
      { dateStyle: 'medium', timeStyle: 'short' });
  } catch { return iso; }
}

async function api(path, opts = {}) {
  const res = await fetch(API + path, {
    headers: { 'Content-Type': 'application/json', ...(opts.headers || {}) },
    ...opts,
  });
  const ct = res.headers.get('content-type') || '';
  const data = ct.includes('application/json') ? await res.json() : null;
  if (!res.ok) {
    const err = new Error(data?.error || res.statusText || `HTTP ${res.status}`);
    err.status = res.status;
    throw err;
  }
  return data;
}

function tagPill(tag) {
  return `<a href="#/?tag=${encodeURIComponent(tag)}"
            class="tag-pill"
            style="--tag-rgb: ${tagColor(tag)}">${esc(tag)}</a>`;
}

function articleCard(a) {
  const tags = (a.tags || []).map(tagPill).join('');
  const scoreBadge = a.score !== undefined
    ? `<span class="text-hot font-medium">релевантность ${a.score}</span>`
    : '';
  return `
    <article class="article-card">
      <a href="#/article/${a.id}" class="block group">
        <h2 class="text-lg font-semibold text-ink group-hover:text-brand transition">
          ${esc(a.title)}
        </h2>
      </a>
      <div class="flex flex-wrap gap-1.5 mt-2">${tags}</div>
      <div class="flex items-center gap-4 mt-3 text-xs text-slate-500">
        <span class="inline-flex items-center gap-1 text-hot">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="currentColor">
            <path d="M12 21s-7-4.5-9-9c-1.5-3.5 1-7 4-7 2 0 3 1 5 3 2-2 3-3 5-3 3 0 5.5 3.5 4 7-2 4.5-9 9-9 9z"/>
          </svg>
          ${a.likes}
        </span>
        <span>${fmtDate(a.created_at)}</span>
        ${scoreBadge}
      </div>
    </article>
  `;
}

function parseRoute() {
  const hash = window.location.hash || '#/';
  const [path, qs] = hash.slice(1).split('?');
  return {
    parts: path.split('/').filter(Boolean),
    params: new URLSearchParams(qs || ''),
  };
}

async function renderRoute() {
  const app = document.getElementById('app');
  const { parts, params } = parseRoute();
  try {
    if (parts.length === 0)              return await renderFeed(app, params);
    if (parts[0] === 'article' && parts[1]) return await renderArticle(app, Number(parts[1]));
    if (parts[0] === 'search')           return await renderSearch(app, params.get('q') || '');
    if (parts[0] === 'new')              return renderNew(app);
    if (parts[0] === 'tags')             return await renderTags(app);
    app.innerHTML = '<div class="text-center text-slate-400 py-12">Страница не найдена</div>';
  } catch (e) {
    app.innerHTML = `
      <div class="bg-red-50 text-red-700 p-4 rounded-lg">
        <strong>Ошибка:</strong> ${esc(e.message)}
      </div>`;
  }
}

async function renderFeed(app, params) {
  const page    = Math.max(1, Number(params.get('page')) || 1);
  const tag     = params.get('tag') || '';
  const sort    = params.get('sort') || 'new';
  const perPage = 10;

  const qs = new URLSearchParams({ page, per_page: perPage, sort });
  if (tag) qs.set('tag', tag);

  const data = await api('/api/articles?' + qs);
  const totalPages = Math.max(1, Math.ceil(data.total / perPage));

  const articlesHtml = data.articles.length
    ? data.articles.map(articleCard).join('')
    : `<div class="text-center text-slate-400 py-12">
         ${tag ? `По тегу «${esc(tag)}» пока ничего нет` : 'Здесь пока пусто'}
       </div>`;

  const sortBtns = [['new', 'Свежие'], ['likes', 'Популярные'], ['title', 'По алфавиту']]
    .map(([k, label]) => {
      const q = new URLSearchParams({ sort: k });
      if (tag) q.set('tag', tag);
      const active = sort === k
        ? 'bg-brand text-white'
        : 'bg-white text-slate-600 border border-slate-200 hover:border-brand';
      return `<a href="#/?${q}" class="px-3 py-1 rounded-full text-sm transition ${active}">${label}</a>`;
    }).join('');

  const tagBadge = tag ? `
    <div class="flex items-center gap-2 mb-4 flex-wrap">
      <span class="text-sm text-slate-500">Фильтр:</span>
      <span class="tag-pill" style="--tag-rgb: ${tagColor(tag)}">${esc(tag)}</span>
      <a href="#/?sort=${sort}" class="text-xs text-slate-400 hover:text-brand">сбросить</a>
    </div>` : '';

  const makePageLink = p => {
    const q = new URLSearchParams({ sort, page: p });
    if (tag) q.set('tag', tag);
    return '#/?' + q;
  };
  const pagination = totalPages > 1 ? `
    <div class="flex items-center justify-center gap-3 mt-8">
      ${page > 1
        ? `<a href="${makePageLink(page - 1)}" class="px-3 py-1.5 bg-white rounded-lg border border-slate-200 hover:border-brand transition">←</a>`
        : '<span class="px-3 py-1.5 text-slate-300">←</span>'}
      <span class="text-sm text-slate-500">${page} / ${totalPages}</span>
      ${page < totalPages
        ? `<a href="${makePageLink(page + 1)}" class="px-3 py-1.5 bg-white rounded-lg border border-slate-200 hover:border-brand transition">→</a>`
        : '<span class="px-3 py-1.5 text-slate-300">→</span>'}
    </div>` : '';

  app.innerHTML = `
    <div class="flex flex-wrap gap-2 mb-4">${sortBtns}</div>
    ${tagBadge}
    <div class="space-y-3">${articlesHtml}</div>
    ${pagination}
  `;
}

async function renderArticle(app, id) {
  const a = await api(`/api/articles/${id}`);
  const tags = (a.tags || []).map(tagPill).join('');

  let related = [];
  try {
    related = (await api(`/api/articles/${id}/related?limit=5`)).related || [];
  } catch { /* не критично */ }

  const relatedHtml = related.length ? `
    <section class="mt-10">
      <h2 class="text-lg font-semibold mb-3">Похожие статьи</h2>
      <div class="space-y-2">
        ${related.map(r => `
          <a href="#/article/${r.id}"
             class="block bg-white rounded-lg p-3 hover:bg-brand/5 border border-slate-100 transition">
            <div class="flex justify-between items-center gap-3">
              <span class="font-medium text-ink">${esc(r.title)}</span>
              <span class="text-xs text-slate-400 shrink-0">${(r.score * 100).toFixed(0)}%</span>
            </div>
          </a>`).join('')}
      </div>
    </section>` : '';

  app.innerHTML = `
    <a href="#/" class="text-sm text-slate-500 hover:text-brand">← К ленте</a>
    <article class="bg-white rounded-xl p-6 md:p-8 mt-4 shadow-sm">
      <h1 class="text-2xl md:text-3xl font-bold text-ink">${esc(a.title)}</h1>
      <div class="flex flex-wrap items-center gap-3 mt-3 text-sm text-slate-500">
        <span>${fmtDate(a.created_at)}</span>
        <div class="flex flex-wrap gap-1.5">${tags}</div>
      </div>
      <div class="article-body mt-6 text-ink">${esc(a.content)}</div>
      <div class="mt-6 flex items-center gap-3">
        <button id="like-btn"
                class="flex items-center gap-2 px-4 py-2 rounded-lg
                       bg-hot/10 text-hot hover:bg-hot/20 transition font-medium">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
            <path d="M12 21s-7-4.5-9-9c-1.5-3.5 1-7 4-7 2 0 3 1 5 3 2-2 3-3 5-3 3 0 5.5 3.5 4 7-2 4.5-9 9-9 9z"/>
          </svg>
          <span id="like-count">${a.likes}</span>
        </button>
      </div>
    </article>
    ${relatedHtml}
  `;

  document.getElementById('like-btn').addEventListener('click', async (ev) => {
    const btn = ev.currentTarget;
    btn.disabled = true;
    try {
      const r = await api(`/api/articles/${id}/like`, { method: 'POST' });
      document.getElementById('like-count').textContent = r.likes;
    } catch (e) {
      alert('Не удалось поставить лайк: ' + e.message);
    } finally {
      btn.disabled = false;
    }
  });
}

async function renderSearch(app, q) {
  document.getElementById('search-input').value = q;
  q = q.trim();
  if (!q) {
    app.innerHTML = `<div class="text-center text-slate-400 py-12">
      Введите запрос в строку поиска сверху</div>`;
    return;
  }

  const data = await api('/api/search?' + new URLSearchParams({ q, page: 1, per_page: 20 }));
  const resultsHtml = data.results.length
    ? data.results.map(articleCard).join('')
    : `<div class="text-center text-slate-400 py-12">
         По запросу «${esc(q)}» ничего не найдено</div>`;

  app.innerHTML = `
    <div class="mb-4">
      <h1 class="text-xl font-semibold">
        Поиск: <span class="text-brand">${esc(q)}</span>
      </h1>
      <p class="text-sm text-slate-500 mt-1">Найдено: ${data.total}</p>
    </div>
    <div class="space-y-3">${resultsHtml}</div>
  `;
}

function renderNew(app) {
  app.innerHTML = `
    <a href="#/" class="text-sm text-slate-500 hover:text-brand">← К ленте</a>
    <form id="new-form" class="bg-white rounded-xl p-6 mt-4 shadow-sm space-y-4">
      <h1 class="text-xl font-semibold">Новая статья</h1>

      <div>
        <label class="block text-sm font-medium mb-1">Заголовок</label>
        <input name="title" required maxlength="200"
               class="w-full px-3 py-2 border border-slate-200 rounded-lg
                      focus:outline-none focus:ring-2 focus:ring-brand/30 focus:border-brand">
      </div>

      <div>
        <label class="block text-sm font-medium mb-1">Текст</label>
        <textarea name="content" required rows="10"
                  class="w-full px-3 py-2 border border-slate-200 rounded-lg
                         focus:outline-none focus:ring-2 focus:ring-brand/30 focus:border-brand"></textarea>
      </div>

      <div>
        <label class="block text-sm font-medium mb-1">Теги (через запятую)</label>
        <input name="tags" placeholder="cpp, stl, algorithms"
               class="w-full px-3 py-2 border border-slate-200 rounded-lg
                      focus:outline-none focus:ring-2 focus:ring-brand/30 focus:border-brand">
      </div>

      <div id="new-error" class="hidden text-red-600 text-sm"></div>

      <button type="submit"
              class="bg-brand text-white px-4 py-2 rounded-lg hover:opacity-90
                     disabled:opacity-50 transition">
        Опубликовать
      </button>
    </form>
  `;

  const form = document.getElementById('new-form');
  const errorEl = document.getElementById('new-error');

  form.addEventListener('submit', async e => {
    e.preventDefault();
    errorEl.classList.add('hidden');
    const fd = new FormData(form);
    const tags = String(fd.get('tags') || '')
      .split(',').map(s => s.trim()).filter(Boolean);

    const btn = form.querySelector('button[type=submit]');
    btn.disabled = true;
    try {
      const article = await api('/api/articles', {
        method: 'POST',
        body: JSON.stringify({
          title: fd.get('title'),
          content: fd.get('content'),
          tags,
        }),
      });
      location.hash = `#/article/${article.id}`;
    } catch (err) {
      errorEl.textContent = err.message;
      errorEl.classList.remove('hidden');
    } finally {
      btn.disabled = false;
    }
  });
}

async function renderTags(app) {
  const data = await api('/api/tags');
  const maxCount = Math.max(1, ...data.tags.map(t => t.count));

  const html = data.tags.length ? data.tags.map(t => {
    const scale = 0.85 + (t.count / maxCount) * 0.55;
    return `
      <a href="#/?tag=${encodeURIComponent(t.name)}"
         class="tag-pill"
         style="--tag-rgb: ${tagColor(t.name)}; font-size: ${scale}rem; padding: 4px 14px">
        ${esc(t.name)}
        <span class="opacity-60">${t.count}</span>
      </a>`;
  }).join('') : `<div class="text-slate-400">Тегов пока нет</div>`;

  app.innerHTML = `
    <h1 class="text-xl font-semibold mb-4">Облако тегов</h1>
    <div class="flex flex-wrap gap-2">${html}</div>
  `;
}

document.getElementById('search-form').addEventListener('submit', e => {
  e.preventDefault();
  const q = document.getElementById('search-input').value.trim();
  if (q) location.hash = `#/search?q=${encodeURIComponent(q)}`;
});

window.addEventListener('hashchange', renderRoute);
renderRoute();

if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('sw.js').catch(() => {});
}
