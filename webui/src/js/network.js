$(function() {
    const iconEye = `
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-eye-fill" viewBox="0 0 16 16">
          <path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0"/>
          <path d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8m8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7"/>
        </svg>
    `;
    const iconEyeSlash = `
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" class="bi bi-eye-slash-fill" viewBox="0 0 16 16">
          <path d="m10.79 12.912-1.614-1.615a3.5 3.5 0 0 1-4.474-4.474l-2.06-2.06C.938 6.278 0 8 0 8s3 5.5 8 5.5a7 7 0 0 0 2.79-.588M5.21 3.088A7 7 0 0 1 8 2.5c5 0 8 5.5 8 5.5s-.939 1.721-2.641 3.238l-2.062-2.062a3.5 3.5 0 0 0-4.474-4.474z"/>
          <path d="M5.525 7.646a2.5 2.5 0 0 0 2.829 2.829zm4.95.708-2.829-2.83a2.5 2.5 0 0 1 2.829 2.829zm3.171 6-12-12 .708-.708 12 12z"/>
        </svg>
    `;

    const $form = $("#network-form-wifi");
    const $btnSave = $("#network-btn-connect");
    const $btnReset = $("#network-btn-reset");
    const $fieldSsid = $("#network-field-ssid");
    const $fieldPassword = $("#network-field-password");
    const $togglePassword = $("#network-toggle-password");
    const $modalConnect = $("#network-modal-connect");
    const $modalRevert = $("#network-modal-revert");
    const bsmodalConnect = new bootstrap.Modal($modalConnect);
    const bsmodalRevert = new bootstrap.Modal($modalRevert);

    function setPasswordVisible(visible) {
        $togglePassword.empty();
        if (visible) {
            $togglePassword.append(iconEye);
            $fieldPassword.attr("type", "text");
        } else {
            $togglePassword.append(iconEyeSlash);
            $fieldPassword.attr("type", "password");
        }
    }

    function togglePasswordVisible() {
        const visible = ($fieldPassword.attr("type") === "text");
        setPasswordVisible(!visible);
    }

    // pull the saved network
    $form.hide();
    $.ajax({
        url: process.env.APP_API_SERVER + "/api/network/get",
        type: "GET",
    }).done(function(data) {
        $form.show();
        $fieldSsid.val(data.ssid);
        $fieldPassword.val(data.password);
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

    setPasswordVisible(false);
    $togglePassword.on("click", function() {
        togglePasswordVisible();
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
