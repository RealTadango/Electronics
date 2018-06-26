showErrorFromResult = function () {
    alert('Error while updating setting');
}

SetSerialSpeed = function () {
    $.ajax('setserialspeed', {
        type: 'POST',
        data: { speed: $('#serialSpeed').val() },
        error: showErrorFromResult
    });
}

SetAP = function () {
    $.ajax('setap', {
        type: 'POST',
        data: { ssid: $('#apSsid').val(), password: $('#apPassword').val() },
        error: showErrorFromResult
    });
}

SetNetwork = function () {
    var row = $(this).closest('tr');
    var ssid = $(row.find('td input')[0]).val();
    var password = $(row.find('td input')[1]).val();

    var index = row.attr('data-index');

    $.ajax('setnetwork', {
        type: 'POST',
        data: { index: index, ssid: ssid, password: password },
        error: showErrorFromResult
    });
}

LoadStatus = function() {
	$.ajax('getstatus', {
        type: 'GET',
        success: function(result)
        {
            SetStatus(result);
        },
        error: function () {
            alert('Error loading status, using dummy');

            SetStatus({
                apIP : '1.2.3.4',
                networkIP : '1.2.3.4'
            });
        }
    });
}

SetStatus = function(status) {
	$('#apIP').text(status.apIP);
	$('#networkIP').text(status.networkIP);
}

LoadConfig = function () {
    $.ajax('getconfig', {
        type: 'GET',
        success: function(result)
        {
            SetConfig(result);
        },
        error: function () {
            alert('Error loading configuration, using dummy');

            SetConfig({
                serialSpeed: 57600,
                apSsid: 'TelnetBridge',
                apPassword: 'TelnetBridge',
                networks: [
                    { ssid: 'dummy', password: 'dummy' },
                    { ssid: '', password: '' },
                    { ssid: '', password: '' },
                    { ssid: '', password: '' },
                    { ssid: '', password: '' }]
            });
        }
    });
}

SetConfig = function (config) {
    $('#serialSpeed').val(config.serialSpeed);
    $('#apSsid').val(config.apSsid);
    $('#apPassword').val(config.apPassword);

    for (var index in config.networks) {
        var network = config.networks[index];

        var row = '<tr data-index="' + index + '">';

        row += '<td>' + (parseInt(index) + 1) + '</td>';
        row += '<td><input type="text" value="' + network.ssid + '" maxlength="50" /></td>';
        row += '<td><input type="text" value="' + network.password + '" maxlength="50" /></td>';
        row += '<td><input type="button" value="Save" /></td>';

        row += '</tr>';
        $('#networks_placeholder tbody').append(row);
    }

    $('#networks_placeholder tbody input:button').click(SetNetwork);
}

$(function () {
    LoadConfig();
	LoadStatus();
    $('#serialPort_save').click(SetSerialSpeed);
    $('#ap_save').click(SetAP);
})