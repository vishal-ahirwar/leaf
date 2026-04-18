const state = { 
  token: localStorage.getItem("leaf_token") || "",
  theme: localStorage.getItem("leaf_theme") || (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light'),
  cachedRecipes: [] // to fall back to when not searching
};

const dom = {
  status: document.getElementById("statusMsg"),
  statusContainer: document.getElementById("loginStatus"),
  recipesBody: document.getElementById("recipesTableBody"),
  remoteUrl: document.getElementById("remoteUrl"),
  loginPanel: document.getElementById("loginPanel"),
  dashboardPanel: document.getElementById("dashboardPanel"),
  themeToggle: document.getElementById("themeToggle"),
  usernameInput: document.getElementById("usernameInput"),
  passwordInput: document.getElementById("passwordInput"),
  loginBtn: document.getElementById("loginBtn"),
  refreshBtn: document.getElementById("refreshBtn"),
  countRecipes: document.getElementById("countRecipes"),
  countRevisions: document.getElementById("countRevisions"),
  countPackages: document.getElementById("countPackages"),
  searchContainer: document.getElementById("searchContainer"),
  searchInput: document.getElementById("searchInput"),
  userContextBtn: document.getElementById("userContextBtn"),
  userModal: document.getElementById("userModal"),
  closeUserModal: document.getElementById("closeUserModal"),
  userTokenBox: document.getElementById("userTokenBox"),
  logoutBtn: document.getElementById("logoutBtn")
};

function initTheme() {
  document.documentElement.setAttribute('data-theme', state.theme);
  updateThemeIcon();
}

function toggleTheme() {
  state.theme = state.theme === 'dark' ? 'light' : 'dark';
  document.documentElement.setAttribute('data-theme', state.theme);
  localStorage.setItem("leaf_theme", state.theme);
  updateThemeIcon();
}

function updateThemeIcon() {
  if (state.theme === 'dark') {
    dom.themeToggle.innerHTML = `<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="5"></circle><line x1="12" y1="1" x2="12" y2="3"></line><line x1="12" y1="21" x2="12" y2="23"></line><line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line><line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line><line x1="1" y1="12" x2="3" y2="12"></line><line x1="21" y1="12" x2="23" y2="12"></line><line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line><line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line></svg>`;
  } else {
    dom.themeToggle.innerHTML = `<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path></svg>`;
  }
}

dom.themeToggle.addEventListener('click', toggleTheme);
initTheme();
dom.remoteUrl.textContent = `${location.origin}`;

function setStatus(message, isError = false) {
  dom.status.textContent = message;
  dom.statusContainer.classList.remove('hidden');
  dom.statusContainer.className = `status-banner ${isError ? 'error' : 'success'}`;
  if (!isError) {
    setTimeout(() => { dom.statusContainer.classList.add('hidden'); }, 4000);
  }
}

async function api(path, options = {}) {
  const headers = new Headers(options.headers || {});
  if (state.token) {
    headers.set("Authorization", `Bearer ${state.token}`);
  }
  const response = await fetch(path, { ...options, headers });
  if (!response.ok) {
    const errText = await response.text();
    throw new Error(errText || response.statusText);
  }
  const text = await response.text();
  return text ? JSON.parse(text) : {};
}

function showDashboard() {
  dom.loginPanel.classList.remove('visible');
  setTimeout(() => {
    dom.loginPanel.classList.add('hidden');
    dom.dashboardPanel.classList.remove('hidden');
    dom.searchContainer.classList.remove('hidden');
    dom.userContextBtn.classList.remove('hidden');
    setTimeout(() => dom.dashboardPanel.classList.add('visible'), 50);
  }, 400); // Wait for fade out
}

function showLogin() {
  dom.dashboardPanel.classList.remove('visible');
  dom.searchContainer.classList.add('hidden');
  dom.userContextBtn.classList.add('hidden');
  setTimeout(() => {
    dom.dashboardPanel.classList.add('hidden');
    dom.loginPanel.classList.remove('hidden');
    setTimeout(() => dom.loginPanel.classList.add('visible'), 50);
  }, 400); // Wait for fade out
}

async function handleLogin() {
  const username = dom.usernameInput.value.trim();
  const password = dom.passwordInput.value;
  if (!username || !password) { setStatus("Provide credentials.", true); return; }
  
  dom.loginBtn.disabled = true;
  dom.loginBtn.innerHTML = `<span class="spinner"></span> Verifying...`;

  try {
    const auth = btoa(`${username}:${password}`);
    const response = await fetch("/api/ui/login", { headers: { Authorization: `Basic ${auth}` } });
    if (!response.ok) {
        const text = await response.text();
        throw new Error(text || response.statusText || "Invalid credentials");
    }
    
    const payload = await response.json();
    state.token = payload.token;
    localStorage.setItem("leaf_token", state.token);
    
    setStatus("Successfully authenticated!");
    await refreshDashboard();
    showDashboard();
  } catch (error) {
    setStatus(error.message, true);
  } finally {
    dom.loginBtn.disabled = false;
    dom.loginBtn.innerHTML = `Access Repository <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="5" y1="12" x2="19" y2="12"></line><polyline points="12 5 19 12 12 19"></polyline></svg>`;
  }
}

function formatTime(isoStr) {
  if (!isoStr || isoStr === "-") return "-";
  return new Date(Date.parse(isoStr)).toLocaleString(undefined, {
    year: 'numeric', month: 'short', day: 'numeric', hour: '2-digit', minute: '2-digit'
  });
}

function expandRowContent(tr, recipeRef) {
  if (tr.nextElementSibling && tr.nextElementSibling.classList.contains("expanded-row")) {
    tr.nextElementSibling.remove();
    tr.style.backgroundColor = "";
    return;
  }
  
  tr.style.backgroundColor = "var(--bg-surface-2)";
  const expandTr = document.createElement("tr");
  expandTr.className = "expanded-row";
  
  const td = document.createElement("td");
  td.colSpan = 3;
  td.innerHTML = `
    <div class="expanded-content">
      <div style="display:flex; justify-content:center; align-items:center; padding: 20px;">
        <span class="spinner spinner-dark"></span>
      </div>
    </div>
  `;
  expandTr.appendChild(td);
  tr.after(expandTr);

  // Fetch full tree via endpoint
  const parseRef = recipeRef.split('@'); // "name/version@user/channel"
  const nv = parseRef[0].split('/');
  const uc = parseRef[1].split('/');
  const basePath = `/v2/conans/${encodeURIComponent(nv[0])}/${encodeURIComponent(nv[1])}/${encodeURIComponent(uc[0])}/${encodeURIComponent(uc[1])}`;

  loadExpandedContent(td.querySelector('.expanded-content'), basePath, recipeRef);
}

async function loadExpandedContent(container, basePath, recipeRef) {
  try {
    const revisionsObj = await api(`${basePath}/revisions`);
    
    let html = `<div class="nested-section"><div class="nested-title">All Revisions</div>`;
    
    for (const rev of revisionsObj.revisions) {
      html += `<div style="padding: 12px; background: var(--bg-surface-2); border-radius: 8px; margin-bottom: 8px;">`;
      html += `<div style="display:flex; justify-content:space-between; margin-bottom: 8px;">`;
      html += `<div><span class="data-tag">${rev.revision}</span> <span class="muted" style="font-size:0.8rem;">${formatTime(rev.time)}</span></div>`;
      
      html += `<button class="action-btn danger-outline" onclick="deleteRevision('${basePath}', '${rev.revision}', '${recipeRef}')">
                <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg> Delete
               </button></div>`;
      
      // Load files and packages in parallel
      const [filesObj, pkgsObj] = await Promise.all([
        api(`${basePath}/revisions/${rev.revision}/files`),
        api(`${basePath}/revisions/${rev.revision}/search`)
      ].map(p => p.catch(e => ({}))));
      
      // Files array
      if (filesObj && filesObj.files) {
        html += `<div style="font-size:0.8rem; margin-top:8px;"><strong>Recipe Files: </strong>`;
        Object.keys(filesObj.files).forEach(f => {
          html += `<span class="data-tag" style="background:var(--bg-main); border-color:var(--border-glass);">${f}</span>`;
        });
        html += `</div>`;
      }
      
      // Packages
      if (pkgsObj && Object.keys(pkgsObj).length > 0) {
        html += `<div style="font-size:0.8rem; margin-top:8px;"><strong>Package Binaries: </strong>`;
        for (const [pkgId, details] of Object.entries(pkgsObj)) {
           html += `<div style="margin-top:4px; padding:8px; border: 1px solid var(--border-light); border-radius: 6px;">
                      <span class="data-tag" style="color:var(--brand)">${pkgId}</span>`;
           if (details.settings) {
             Object.entries(details.settings).forEach(([k,v]) => html += `<span class="data-tag" style="font-size:0.7rem">${k}=${v}</span>`);
           }
           html += `</div>`;
        }
        html += `</div>`;
      }
      html += `</div>`;
    }
    
    html += `</div>`;
    container.innerHTML = html;
  } catch (error) {
    container.innerHTML = `<div class="status-banner error" style="position:relative; transform:none; opacity:1;"><span class="text-danger">${error.message}</span></div>`;
  }
}

window.deleteRevision = async (basePath, rev, refName) => {
  if (confirm(`Irreversibly delete revision ${rev} for ${refName}?`)) {
    try {
      await api(`${basePath}/revisions/${rev}`, { method: "DELETE" });
      setStatus(`Deleted ${refName}#${rev.substring(0,8)}`);
      await refreshDashboard();
    } catch(e) {
      setStatus(`Delete failed: ${e.message}`, true);
    }
  }
};

function createTableRow(recipeRef, revCount, latestRev, latestTime) {
  const row = document.createElement("tr");

  row.innerHTML = `
    <td>
      <div class="table-ref">
        <svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="ref-icon"><rect x="2" y="7" width="20" height="14" rx="2" ry="2"></rect><path d="M16 21V5a2 2 0 0 0-2-2h-4a2 2 0 0 0-2 2v16"></path></svg>
        <span class="ref-text">${recipeRef}</span>
      </div>
    </td>
    <td>
      <div class="rev-badge">${revCount}</div>
    </td>
    <td>
      <div style="display:flex; flex-direction:column;">
        <span style="font-family:monospace; font-size: 0.9rem; color:var(--text-main);">${latestRev.substring(0, 16)}${latestRev.length > 16 ? '...' : ''}</span>
        <span style="font-size: 0.8rem; color:var(--text-muted);">${formatTime(latestTime)}</span>
      </div>
    </td>
  `;

  row.addEventListener("click", () => expandRowContent(row, recipeRef));
  return row;
}

function animateValue(obj, start, end, duration) {
  let startTimestamp = null;
  const step = (timestamp) => {
    if (!startTimestamp) startTimestamp = timestamp;
    const progress = Math.min((timestamp - startTimestamp) / duration, 1);
    obj.innerHTML = Math.floor(progress * (end - start) + start);
    if (progress < 1) window.requestAnimationFrame(step);
  };
  window.requestAnimationFrame(step);
}

async function refreshDashboard() {
  dom.refreshBtn.classList.add('spinning');
  
  try {
    const summary = await api("/api/ui/summary");
    const recipes = await api("/api/ui/recipes");
    state.cachedRecipes = recipes.recipes; // cache for UI

    animateValue(dom.countRecipes, parseInt(dom.countRecipes.textContent) || 0, summary.recipes, 800);
    animateValue(dom.countRevisions, parseInt(dom.countRevisions.textContent) || 0, summary.recipe_revisions, 800);
    animateValue(dom.countPackages, parseInt(dom.countPackages.textContent) || 0, summary.packages, 800);

    renderTable(state.cachedRecipes);
  } catch (error) {
    if (error.message.includes("401") || error.message.includes("Unauthorized")) {
      setStatus("Session expired. Please log in again.", true);
      state.token = ""; localStorage.removeItem("leaf_token");
      showLogin();
    } else {
      setStatus(error.message, true);
    }
  } finally {
    setTimeout(() => dom.refreshBtn.classList.remove('spinning'), 500);
  }
}

function renderTable(recipeList) {
  dom.recipesBody.innerHTML = "";
  if (recipeList.length === 0) {
    dom.recipesBody.innerHTML = `<tr><td colspan="3" class="empty-state">
      <svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="11" cy="11" r="8"></circle><line x1="21" y1="21" x2="16.65" y2="16.65"></line></svg>
      <p style="color:var(--text-main); font-weight:600; font-family:'Space Grotesk'">No packages found</p>
      <p class="muted">Your repository registry is currently empty or query yielded no results.</p>
    </td></tr>`;
  } else {
    for (const recipe of recipeList) {
      if(recipe.reference) { // came from full UI objects
        const latest = recipe.revisions[0] || { revision: "-", time: "-" };
        dom.recipesBody.appendChild(createTableRow(recipe.reference, recipe.revisions.length, latest.revision, latest.time));
      } else { // came from standard search API strings
        // Just render placeholder, user can expand to fetch
        dom.recipesBody.appendChild(createTableRow(recipe, "?", "-", "-"));
      }
    }
  }
}

// Global Search Debouncer
let searchTimeout = null;
dom.searchInput.addEventListener("input", (e) => {
  clearTimeout(searchTimeout);
  const q = e.target.value.trim();
  searchTimeout = setTimeout(async () => {
    if(!q) { renderTable(state.cachedRecipes); return; }
    try {
      const res = await api(`/v2/conans/search?q=${encodeURIComponent(q)}*`);
      // res.results is an array of strings e.g. ["pkg/1.0@user/stable"]
      renderTable(res.results || []);
    } catch(err) { setStatus(err.message, true); }
  }, 350);
});

// UI Modal Handlers
dom.userContextBtn.addEventListener("click", async () => {
  dom.userModal.classList.add("active");
  try {
     const check = await api("/v2/users/check_credentials"); // verify token active
     dom.userTokenBox.textContent = state.token;
  } catch(e) { dom.userTokenBox.textContent = `Error: ${e.message}`; }
});
dom.closeUserModal.addEventListener("click", () => dom.userModal.classList.remove("active"));
dom.logoutBtn.addEventListener("click", () => {
  state.token = ""; localStorage.removeItem("leaf_token");
  dom.userModal.classList.remove("active");
  showLogin();
});

// Init
dom.loginBtn.addEventListener("click", handleLogin);
dom.passwordInput.addEventListener('keypress', (e) => { if (e.key === 'Enter') handleLogin(); });
dom.refreshBtn.addEventListener("click", refreshDashboard);

if (state.token) {
  showDashboard();
  refreshDashboard().catch(e => showLogin());
} else {
  showLogin();
}
