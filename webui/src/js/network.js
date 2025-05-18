$(function() {
    const $form = $("#network-form-wifi");
    const $btnSave = $("#network-btn-connect");
    const $btnReset = $("#network-btn-reset");
    const $fieldSsid = $("#network-field-ssid");
    const $fieldPassword = $("#network-field-password");
    const $modalConnect = $("#network-modal-connect");
    const $modalRevert = $("#network-modal-revert");
    const bsmodalConnect = new bootstrap.Modal($modalConnect);
    const bsmodalRevert = new bootstrap.Modal($modalRevert);

    function setEnableState(enable) {
        $.each([$btnSave, $btnReset, $fieldSsid, $fieldPassword], function(i, e) {
            e.prop("disabled", !enable);
        });
    }

    // pull the saved network
    setEnableState(false);
    $.ajax({
        url: process.env.APP_API_SERVER + "/api/network/get",
        type: "GET",
    }).done(function(data) {
        $fieldSsid.val(data.ssid);
        $fieldPassword.val(data.password);
    }).always(function() {
        setEnableState(true);
    });

    $btnSave.on("click", function() {
        $form.addClass('was-validated');
        if (!$form.get(0).checkValidity())
            return false;
        bsmodalConnect.show();
    });

    $btnReset.on("click", function() {
        bsmodalRevert.show();
    });

    $modalConnect.find(".btn-primary").on("click", function() {
        bsmodalConnect.hide();
        $.ajax({
            url: process.env.APP_API_SERVER + "/api/network/save",
            type: "POST",
            contentType: 'application/json',
            data: JSON.stringify({
                "ssid": $fieldSsid.val(),
                "password": $fieldPassword.val(),
            }),
        });
    });

    $modalRevert.find(".btn-primary").on("click", function() {
        bsmodalRevert.hide();
        $.ajax({
            url: process.env.APP_API_SERVER + "/api/network/reset",
            type: "POST",
            contentType: 'application/json',
        });
    });
});
