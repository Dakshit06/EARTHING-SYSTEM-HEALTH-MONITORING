document.addEventListener('DOMContentLoaded', () => {
    // === DOM Elements ===
    const valSoil = document.getElementById('val-soil');
    const barSoil = document.getElementById('bar-soil');
    const cardSoil = document.getElementById('card-soil');

    const valVoltage = document.getElementById('val-voltage');
    const barVoltage = document.getElementById('bar-voltage');
    const cardVoltage = document.getElementById('card-voltage');

    const statusHero = document.getElementById('status-hero');
    const mainStatusRing = document.getElementById('main-status-ring');
    const mainStatusIcon = document.getElementById('main-status-icon');
    const mainStatusTitle = document.getElementById('main-status-title');
    const mainStatusDesc = document.getElementById('main-status-desc');
    const statusBadgeText = document.getElementById('header-status-text');
    const statusDot = document.querySelector('.status-dot');

    // Alert indicators
    const redLedIndicator = document.getElementById('red-led-indicator');
    const buzzerIndicator = document.getElementById('buzzer-indicator');
    const alertIndicators = document.getElementById('alert-indicators');

    const clockElement = document.getElementById('clock');

    // === Chart Setup ===
    const ctx = document.getElementById('voltageChart').getContext('2d');
    const voltageChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: Array(20).fill(''),
            datasets: [{
                label: 'AC Voltage (V)',
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

    // === Logic Constants (matching Arduino code) ===
    const SOIL_THRESHOLD = 25;      // moisturePercent >= 25
    const VOLT_MIN = 2.55;          // acVoltage >= 2.55
    const VOLT_MAX = 2.61;          // acVoltage <= 2.61

    function updateClock() {
        const now = new Date();
        clockElement.textContent = now.toLocaleTimeString();
    }

    // Check earthing status based on Arduino logic
    function checkEarthingStatus(moisture, voltage) {
        // EARTHING LOGIC from Arduino:
        // earthingGood = (moisturePercent >= 25 && acVoltage >= 2.55 && acVoltage <= 2.61)
        const soilGood = moisture >= SOIL_THRESHOLD;
        const voltGood = voltage >= VOLT_MIN && voltage <= VOLT_MAX;
        const earthingGood = soilGood && voltGood;
        
        return { earthingGood, soilGood, voltGood };
    }

    async function fetchData() {
        try {
            const response = await fetch('/data');
            if (!response.ok) throw new Error('Network response was not ok');
            const data = await response.json();

            // Expected JSON: { "moisture": 45, "voltage": 2.58 }
            const { earthingGood, soilGood, voltGood } = checkEarthingStatus(data.moisture, data.voltage);
            
            updateDashboard(data.moisture, data.voltage, earthingGood, soilGood, voltGood);

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

    function updateDashboard(moistureVal, voltageVal, earthingGood, isSoilGood, isVoltGood) {
        // Round values
        const dispMoisture = Math.round(moistureVal);
        const dispVoltage = voltageVal.toFixed(2);

        // Update DOM Text
        valSoil.textContent = dispMoisture;
        valVoltage.textContent = dispVoltage;

        // Update Bars
        barSoil.style.width = `${dispMoisture}%`;

        // Voltage bar scaled to show position within range
        const voltPercent = Math.min(100, Math.max(0, ((voltageVal - 2.0) / 1.0) * 100));
        barVoltage.style.width = `${voltPercent}%`;

        updateStatusUI(earthingGood, isSoilGood, isVoltGood);
        updateChart(voltageVal, earthingGood);
    }

    function updateStatusUI(earthingGood, soilGood, voltGood) {
        if (earthingGood) {
            // EARTHING GOOD - matches Arduino display
            mainStatusRing.className = 'status-ring safe';
            mainStatusIcon.className = 'fa-solid fa-check-circle';
            mainStatusTitle.textContent = 'STATUS:';
            mainStatusDesc.textContent = 'EARTHING GOOD';
            mainStatusDesc.className = 'status-label good';
            
            // Hide alert indicators when earthing is good
            alertIndicators.classList.remove('active');
            redLedIndicator.classList.remove('on');
            buzzerIndicator.classList.remove('on');
            statusHero.classList.remove('alert-mode');
        } else {
            // EARTHING BAD - matches Arduino display (ALERT!)
            mainStatusRing.className = 'status-ring danger';
            mainStatusIcon.className = 'fa-solid fa-triangle-exclamation';
            mainStatusTitle.textContent = 'ALERT!';
            mainStatusDesc.textContent = 'EARTHING BAD';
            mainStatusDesc.className = 'status-label bad';
            
            // Show alert indicators (RED LED + BUZZER active)
            alertIndicators.classList.add('active');
            redLedIndicator.classList.add('on');
            buzzerIndicator.classList.add('on');
            statusHero.classList.add('alert-mode');
        }

        // Card Highlights for Soil
        if (soilGood) {
            valSoil.style.color = 'var(--accent-success)';
            barSoil.style.backgroundColor = 'var(--accent-success)';
            cardSoil.classList.remove('warning');
        } else {
            valSoil.style.color = 'var(--accent-warning)';
            barSoil.style.backgroundColor = 'var(--accent-warning)';
            cardSoil.classList.add('warning');
        }

        // Card Highlights for Voltage
        if (voltGood) {
            valVoltage.style.color = 'var(--accent-success)';
            barVoltage.style.backgroundColor = 'var(--accent-success)';
            cardVoltage.classList.remove('danger');
        } else {
            valVoltage.style.color = 'var(--accent-danger)';
            barVoltage.style.backgroundColor = 'var(--accent-danger)';
            cardVoltage.classList.add('danger');
        }
    }

    function updateChart(newVolt, earthingGood) {
        // Push new data
        const data = voltageChart.data.datasets[0].data;
        data.shift();
        data.push(newVolt);

        // Dynamic color for chart line based on earthing status
        voltageChart.data.datasets[0].borderColor = earthingGood ? '#10b981' : '#ef4444';
        voltageChart.data.datasets[0].backgroundColor = earthingGood ? 'rgba(16, 185, 129, 0.1)' : 'rgba(239, 68, 68, 0.1)';

        voltageChart.update();
    }

    // Start Loops
    setInterval(updateClock, 1000);
    setInterval(fetchData, 1000); // Poll every second

    // Initial call
    updateClock();
    fetchData();
});
