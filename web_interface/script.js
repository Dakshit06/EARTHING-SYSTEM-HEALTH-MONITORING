document.addEventListener('DOMContentLoaded', () => {
    // === DOM Elements ===
    const valSoil = document.getElementById('val-soil');
    const barSoil = document.getElementById('bar-soil');
    const cardSoil = document.getElementById('card-soil');

    const valVoltage = document.getElementById('val-voltage');
    const barVoltage = document.getElementById('bar-voltage');
    const cardVoltage = document.getElementById('card-voltage'); // Fixed ID reference

    const mainStatusRing = document.getElementById('main-status-ring');
    const mainStatusIcon = document.getElementById('main-status-icon');
    const mainStatusTitle = document.getElementById('main-status-title');
    const mainStatusDesc = document.getElementById('main-status-desc');
    const statusBadgeText = document.getElementById('header-status-text');
    const statusDot = document.querySelector('.status-dot');

    const clockElement = document.getElementById('clock');

    // === Chart Setup ===
    const ctx = document.getElementById('voltageChart').getContext('2d');
    const voltageChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: Array(20).fill(''),
            datasets: [{
                label: 'Leakage Voltage (V)',
                data: Array(20).fill(0),
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                borderWidth: 2,
                fill: true,
                tension: 0.4,
                pointRadius: 0
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: false,
                    suggestedMin: 2.4,
                    suggestedMax: 2.8,
                    grid: {
                        color: 'rgba(255, 255, 255, 0.05)'
                    },
                    ticks: {
                        color: '#94a3b8'
                    }
                },
                x: {
                    grid: {
                        display: false
                    }
                }
            },
            plugins: {
                legend: {
                    display: false
                }
            },
            animation: false
        }
    });

    // === Simulation State ===
    // We will simulate values that drift around the "Good" range but occasionally spike to "Bad" for demo purposes.
    let moisture = 35;
    let voltage = 2.58;
    let tickCount = 0;


    // === Logic Constants ===
    const SOIL_THRESHOLD = 25;
    const VOLT_MIN = 2.55;
    const VOLT_MAX = 2.61;

    function updateClock() {
        const now = new Date();
        clockElement.textContent = now.toLocaleTimeString();
    }

    async function fetchData() {
        try {
            const response = await fetch('/data');
            if (!response.ok) throw new Error('Network response was not ok');
            const data = await response.json();

            // Expected JSON: { "moisture": 45, "voltage": 2.58, "soilRaw": 12000, "voltRaw": 2200, "soilGood": true, "voltGood": true }

            updateDashboard(data.moisture, data.voltage, data.soilGood, data.voltGood);

            // Update connection status
            statusBadgeText.textContent = 'CONNECTED';
            statusBadgeText.style.color = 'var(--accent-success)';
            statusDot.style.backgroundColor = 'var(--accent-success)';

        } catch (error) {
            console.error('Fetch error:', error);
            // Show disconnected status
            statusBadgeText.textContent = 'DISCONNECTED';
            statusBadgeText.style.color = 'var(--text-secondary)';
            statusDot.style.backgroundColor = 'var(--text-secondary)';
        }
    }

    function updateDashboard(moistureVal, voltageVal, isSoilGood, isVoltGood) {
        // Round values
        const dispMoisture = Math.round(moistureVal);
        const dispVoltage = voltageVal.toFixed(2);

        // Update DOM Text
        valSoil.textContent = dispMoisture;
        valVoltage.textContent = dispVoltage;

        // Update Bars
        barSoil.style.width = `${dispMoisture}%`;

        const voltPercent = Math.min(100, (voltageVal / 3.3) * 100);
        barVoltage.style.width = `${voltPercent}%`;

        // Check Status Logic
        const isSystemSafe = isSoilGood && isVoltGood;

        updateStatusUI(isSystemSafe, isSoilGood, isVoltGood);
        updateChart(voltageVal, isSystemSafe);
    }

    function updateStatusUI(safe, soilGood, voltGood) {
        if (safe) {
            // Main Status
            mainStatusRing.className = 'status-ring safe';
            mainStatusIcon.className = 'fa-solid fa-check-circle';
            mainStatusTitle.textContent = 'System Safe';
            mainStatusDesc.textContent = 'Grounding effective. No leakage detected.';
        } else {
            // Main Status
            mainStatusRing.className = 'status-ring danger';
            mainStatusIcon.className = 'fa-solid fa-triangle-exclamation';
            mainStatusTitle.textContent = 'Check Earthing';

            let issues = [];
            if (!soilGood) issues.push("Low Moisture");
            if (!voltGood) issues.push("Voltage Leak");
            mainStatusDesc.textContent = `Alert: ${issues.join(' & ')}`;
        }

        // Card Highlights
        if (soilGood) {
            valSoil.style.color = 'var(--accent-success)';
            barSoil.style.backgroundColor = 'var(--accent-success)';
        } else {
            valSoil.style.color = 'var(--accent-warning)';
            barSoil.style.backgroundColor = 'var(--accent-warning)';
        }

        if (voltGood) {
            valVoltage.style.color = 'var(--accent-success)';
            barVoltage.style.backgroundColor = 'var(--accent-success)';
        } else {
            valVoltage.style.color = 'var(--accent-danger)';
            barVoltage.style.backgroundColor = 'var(--accent-danger)';
        }
    }

    function updateChart(newVolt, isSafe) {
        // Push new data
        const data = voltageChart.data.datasets[0].data;
        data.shift();
        data.push(newVolt);

        // Dynamic color for chart line based on safety
        voltageChart.data.datasets[0].borderColor = isSafe ? '#10b981' : '#ef4444';
        voltageChart.data.datasets[0].backgroundColor = isSafe ? 'rgba(16, 185, 129, 0.1)' : 'rgba(239, 68, 68, 0.1)';

        voltageChart.update();
    }

    // Start Loops
    setInterval(updateClock, 1000);
    setInterval(fetchData, 1000); // Poll every second

    // Initial call
    updateClock();
    fetchData();
});
