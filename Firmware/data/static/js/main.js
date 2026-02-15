(() => {
  const root = document.getElementById("root");

  const elements = {
    info: document.createElement("div"),
    grid: document.createElement("div"),
    error: document.createElement("p"),
  };

  const render = (data) => {
    elements.info.innerHTML = `
      <h1>Open Flat Panel</h1>
      <p>Firmware API status endpoint is responding.</p>
    `;

    elements.grid.className = "status-grid";
    elements.grid.innerHTML = `
      <div>Uptime: <span>${Math.round(data.uptimeMs / 1000)}s</span></div>
      <div>Stepper running: <span>${data.stepperRunning}</span></div>
      <div>Current position: <span>${data.currentPosition}</span></div>
      <div>Target position: <span>${data.targetPosition}</span></div>
      <div>Remaining distance: <span>${data.remainingDistance}</span></div>
      <div>Max speed: <span>${data.maxSpeed}</span></div>
      <div>Max acceleration: <span>${data.maxAcceleration}</span></div>
      <div>Microsteps: <span>${data.microsteps}</span></div>
      <div>RMS current: <span>${data.rmsCurrent}</span></div>
      <div>End switch active: <span>${data.endSwitchActive}</span></div>
    `;

    root.replaceChildren(elements.info, elements.grid);
  };

  const renderError = (error) => {
    elements.error.textContent = `Failed to load status: ${error}`;
    root.replaceChildren(elements.error);
  };

  const load = () => {
    fetch("/api/status")
      .then((res) => {
        if (!res.ok) {
          throw new Error(`HTTP ${res.status}`);
        }
        return res.json();
      })
      .then(render)
      .catch((err) => {
        renderError(err.message || "unknown error");
      });
  };

  load();
  setInterval(load, 5000);
})();

