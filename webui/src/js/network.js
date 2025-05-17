$(function() {
    const $form = $("#network-form-wifi");
    const $modalConnect = $("#network-modal-connect");
    const $modalRevert = $("#network-modal-revert");
    const bsmodalConnect = new bootstrap.Modal($modalConnect);
    const bsmodalRevert = new bootstrap.Modal($modalRevert);

    $("#network-btn-connect").on("click", function() {
        $form.addClass('was-validated');
        if (!$form.get(0).checkValidity())
            return false;
        bsmodalConnect.show();
    });

    $("#network-btn-reset").on("click", function() {
        bsmodalRevert.show();
    });

    $modalConnect.find(".btn-primary").on("click", function() {
        bsmodalConnect.hide();
        $.ajax({
            url: process.env.APP_API_SERVER + "/api/network/save",
            type: "POST",
            contentType: 'application/json',
            data: JSON.stringify({
                "ssid": $("#network-field-ssid").val(),
                "password": $("#network-field-password").val(),
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
