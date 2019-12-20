showErrorFromResult = function () {
    alert('Error while updating setting');
}

LoadStatus = function() {
	$.ajax('getstatus', {
        type: 'GET',
        success: function(result)
        {
            SetStatus(result);
        },
        error: showErrorFromResult
    });
	
	setTimeout(LoadStatus, 2000);
}

SetStatus = function(status) {
	$('#apIP').text(status.apIP);
	$('#networkIP').text(status.networkIP);
	$('#networkRSSI').text(status.rssi);
}

LoadConfig = function () {
    $.ajax('getconfig', {
        type: 'GET',
        success: function(result)
        {
            SetConfig(result);
        },
        error: showErrorFromResult
    });
}

SetConfig = function (config) {
	$('#serialSpeed').val(config.serialSpeed);
	$('#apPassword').val(config.apPassword);
	
	if(!config.networks) {
		AddNetwork();
	} else {
		for (var index in config.networks) {
			var network = config.networks[index];
			var row = '<tr>';

			row += '<td><input type="text" class="ssid" value="' + network.ssid + '" maxlength="50" /></td>';
			row += '<td><input type="password" class="password" value="' + network.password + '" maxlength="50" /></td>';
			row += '<td><input type="button" class="remove" value="Remove" /></td>';

			row += '</tr>';
			$('#networks_placeholder tbody').append(row);
		}
	}

    $('#networks_placeholder tbody input.remove').click(function(){
		$(this).closest('tr').remove()
	});
}

Save = function () {
	var data = {
		serialSpeed: $('#serialSpeed').val(),
		apPassword: $('#apPassword').val(),
		networks: []
	};
	
	$('#networks_placeholder tbody tr').each(function(){
		var row = $(this);
		var ssid = row.find('.ssid').val();
		var password = row.find('.password').val();
		
		data.networks.push({ ssid: ssid, password: password});
	});

    $.ajax('setconfig', {
        type: 'POST',
        data: JSON.stringify(data),
        error: showErrorFromResult
    });
}

AddNetwork = function() {
	var row = '<tr>';

	row += '<td><input type="text" class="ssid" maxlength="50" /></td>';
	row += '<td><input type="password" class="password" maxlength="50" /></td>';
	row += '<td><input type="button" class="remove" value="Remove" /></td>';

	row += '</tr>';
	row = $(row);
	$('#networks_placeholder tbody').append(row);
	
	row.find('input.remove').click(function(){
		$(this).closest('tr').remove()
	});
}

$(function () {
    LoadConfig();
	LoadStatus();
	$('#add').click(AddNetwork);
	$('#save').click(Save);
})